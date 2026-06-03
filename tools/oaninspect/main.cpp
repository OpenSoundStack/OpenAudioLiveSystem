// oaninspect — passive OAN wire sniffer.
//
// Connects to sim_switch as a "promiscuous" client and prints every frame
// fanned out by the daemon, decoded per EtherType. Supports interactive
// filtering, pause + scroll-back, and an own-format pcap record/replay.
//
// Intentionally single-file for now: the TUI, ring buffer, and main loop
// are tightly coupled and would gain nothing from a multi-TU split.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

#include "sim_proto.h"
#include "Filter.h"
#include "Decoder.h"

namespace {

constexpr const char* DEFAULT_SOCKET_PATH = "/tmp/osst-sim-default.sock";
constexpr uint32_t    MAX_FRAME_PAYLOAD   = 8192;     // matches sim_switch
// Ring is big enough that ~10s of dense non-audio traffic (sync at a few
// Hz × small peer count + control bursts) survives even with audio also
// flowing — but the audio-skip-at-ingest path below means audio normally
// doesn't compete for ring slots unless the user explicitly filtered it in.
constexpr size_t      DEFAULT_BUFFER      = 100000;
constexpr uint16_t    INSPECTOR_UID       = 0xFFFE;

// Custom record format for --pcap and --replay. NOT libpcap-compatible.
// File starts with this magic + version byte. Then a stream of:
//   uint64_t timestamp_ms
//   SimFrame   header  (sizeof(SimFrame) bytes)
//   uint8_t    payload[header.payload_len]
constexpr char OSTPCAP_MAGIC[7] = {'O','S','T','P','C','A','P'};
constexpr uint8_t OSTPCAP_VERSION = 2;

std::atomic<bool> g_shutdown_requested{false};
void on_signal(int) { g_shutdown_requested = true; }

uint64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
uint64_t wall_now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

struct Args {
    std::string socket_path = DEFAULT_SOCKET_PATH;
    std::string filter_expr;            // empty → audio-suppressed default
    bool        filter_explicit{false}; // true if user passed --filter
    bool        hex{false};
    std::string pcap_path;
    std::string replay_path;
    size_t      buffer{DEFAULT_BUFFER};
};

void print_help() {
    std::cout
        << "oaninspect — passive OAN wire sniffer (talks to sim_switch)\n"
        << "Usage: oaninspect [OPTIONS]\n"
        << "  --socket-path PATH   default " << DEFAULT_SOCKET_PATH << "\n"
        << "  --filter EXPR        e.g. 'ethertype=control,disco' or 'peer=42'\n"
        << "                       keys: ethertype, src, dst, peer, uid\n"
        << "                       (no --filter → audio is suppressed; explicit\n"
        << "                       empty --filter '' = show everything)\n"
        << "  --hex                also dump payload bytes (first 48B)\n"
        << "  --buffer N           ring buffer size for pause/scroll (default "
                                << DEFAULT_BUFFER << ")\n"
        << "  --pcap FILE          record frames to FILE (own format, not libpcap)\n"
        << "  --replay FILE        read FILE instead of connecting to a daemon\n"
        << "Keys at runtime: Space=pause  j/k=scroll  Ctrl-D/U=½page  g/G=top/bottom\n"
        << "                 / = re-enter filter  h=toggle hex  q=quit\n";
}

Args parse_args(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        auto need = [&](const char* opt) {
            if (i + 1 >= argc) {
                std::cerr << "oaninspect: " << opt << " requires a value\n";
                std::exit(2);
            }
            return std::string(argv[++i]);
        };
        if      (s == "--socket-path") a.socket_path = need("--socket-path");
        else if (s == "--filter")    { a.filter_expr = need("--filter"); a.filter_explicit = true; }
        else if (s == "--hex")         a.hex = true;
        else if (s == "--buffer")      a.buffer = std::strtoul(need("--buffer").c_str(), nullptr, 0);
        else if (s == "--pcap")        a.pcap_path = need("--pcap");
        else if (s == "--replay")      a.replay_path = need("--replay");
        else if (s == "-h" || s == "--help") { print_help(); std::exit(0); }
        else {
            std::cerr << "oaninspect: unknown arg '" << s << "'\n";
            std::exit(2);
        }
    }
    if (a.buffer == 0) a.buffer = DEFAULT_BUFFER;
    return a;
}

// One ring-buffer entry. Owns a copy of the payload bytes so the original
// recv buffer can be reused immediately on the hot path.
struct Entry {
    uint64_t   wall_ms;
    uint16_t   ethertype;
    uint16_t   src_uid;
    uint16_t   dst_uid;
    uint8_t    etype_idx;
    std::vector<uint8_t> payload;  // raw bytes the switch fanned out
};

class Ring {
public:
    explicit Ring(size_t cap) : m_cap(cap) {}
    void push(Entry e) {
        if (m_buf.size() == m_cap) m_buf.pop_front();
        m_buf.push_back(std::move(e));
    }
    size_t size() const { return m_buf.size(); }
    const Entry& at(size_t i) const { return m_buf[i]; }
private:
    size_t m_cap;
    std::deque<Entry> m_buf;
};

int open_socket_or_die(const std::string& path) {
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "oaninspect: socket() failed: " << ::strerror(errno) << "\n";
        std::exit(1);
    }
    sockaddr_un a{};
    a.sun_family = AF_UNIX;
    if (path.size() >= sizeof(a.sun_path)) {
        std::cerr << "oaninspect: socket path too long\n";
        std::exit(1);
    }
    std::strncpy(a.sun_path, path.c_str(), sizeof(a.sun_path)-1);
    if (::connect(fd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) < 0) {
        std::cerr << "oaninspect: connect to " << path
                  << " failed (is sim_switch running?): "
                  << ::strerror(errno) << "\n";
        std::exit(1);
    }

    SimHello h{
        SIM_MAGIC,
        SIM_VERSION,
        0,
        0,                  // ethertype — ignored by switch for promiscuous
        INSPECTOR_UID,
        SIM_HELLO_PROMISCUOUS
    };
    if (::send(fd, &h, sizeof(h), 0) != (ssize_t)sizeof(h)) {
        std::cerr << "oaninspect: hello send failed: " << ::strerror(errno) << "\n";
        std::exit(1);
    }

    int flags = ::fcntl(fd, F_GETFL, 0);
    ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return fd;
}

// Place stdin in cbreak (raw-ish) mode so we get keystrokes without Enter.
// Restored on exit via atexit + RAII shutdown.
struct termios g_orig_termios{};
bool g_termios_saved = false;
void restore_termios() {
    if (g_termios_saved) ::tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);
}
void enable_raw_stdin() {
    if (!::isatty(STDIN_FILENO)) return;
    ::tcgetattr(STDIN_FILENO, &g_orig_termios);
    g_termios_saved = true;
    std::atexit(restore_termios);
    struct termios t = g_orig_termios;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 0;
    ::tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// ANSI helpers.
void clear_screen()           { std::cout << "\033[2J\033[H"; }
void move_cursor(int row, int col) { std::cout << "\033[" << row << ";" << col << "H"; }
void clear_line_to_end()      { std::cout << "\033[K"; }

int term_rows() {
    // Lazy: a fixed default works fine; real terminals all support \033[18t
    // or TIOCGWINSZ, but adding either bloats this for marginal gain. The
    // tail-and-status layout is tolerant of a wrong height.
    struct winsize ws{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) return ws.ws_row;
    return 30;
}
int term_cols() {
    struct winsize ws{};
    if (::ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) return ws.ws_col;
    return 120;
}

struct UiState {
    Filter   filter;
    bool     paused{false};
    bool     hex{false};
    bool     color{true};
    // Scroll offset in matching-entries from the bottom. 0 = follow newest.
    int      scroll_offset{0};
    // Counter for status bar suppression hint.
    uint64_t audio_seen{0};
    uint64_t audio_first_ms{0};
    // Frames the wire delivered while pause was active. Resets on unpause
    // so the count reflects "what you'd see if you unpause right now."
    uint64_t paused_dropped{0};

    // ---- diff-render state -----------------------------------------------
    // Snapshot of each rendered line. render() compares against this and
    // only emits ANSI for rows whose content actually changed. Eliminates
    // the full-screen redraw flicker.
    std::vector<std::string> prev_lines;
    std::string              prev_status;
    int                      prev_rows{0};
    int                      prev_cols{0};
    bool                     needs_full_redraw{true};
};

// Compact, always-visible key cheat-sheet rendered above the status bar.
// Kept in one place so adding/removing a binding only needs editing here.
constexpr const char* KEY_HELP =
    "[Space]pause  [j/k]scroll  [Ctrl-D/U] half-pg  [g/G]top/end  [/]filter  [h]hex  [q]quit";

// Render the visible portion of the buffer + key-help + status bar.
//
// Strategy: each refresh builds the full vector of lines we WANT on the
// screen, then diffs against `st.prev_lines` and only emits ANSI for rows
// whose content differs. Avoids the clear-screen flicker that made the
// old version look like it never stopped scrolling.
void render(const Ring& ring, UiState& st, const Args& args) {
    int rows = term_rows();
    // Last row = status bar; row above it = key-help footer. Both fixed.
    int body_rows = rows - 2;
    if (body_rows < 1) body_rows = 1;
    int cols = term_cols();

    // Force a clean redraw if the terminal resized or this is the first
    // paint. Otherwise leftover content from the previous geometry would
    // get stranded on screen.
    if (rows != st.prev_rows || cols != st.prev_cols) {
        st.needs_full_redraw = true;
    }

    // Collect matching entries (indices into ring) up to what we can render
    // from the current scroll position.
    std::vector<size_t> matches;
    matches.reserve(ring.size());
    for (size_t i = 0; i < ring.size(); ++i) {
        const auto& e = ring.at(i);
        if (st.filter.match(e.etype_idx, e.src_uid, e.dst_uid)) {
            matches.push_back(i);
        }
    }

    // Determine which slice we show: tail (paused or unpaused, scroll=0)
    // shows the LAST body_rows matches; scrolling pulls the window up.
    int end = (int)matches.size() - st.scroll_offset;
    int start = end - body_rows;
    if (end > (int)matches.size()) end = (int)matches.size();
    if (start < 0) start = 0;
    if (end < start) end = start;

    DecodeOpts opts{st.hex, st.color};

    // Build the vector of lines we want on screen this tick. Empty strings
    // represent blank body rows.
    std::vector<std::string> new_lines(body_rows);
    int slot = 0;
    for (int i = start; i < end && slot < body_rows; ++i) {
        const auto& e = ring.at(matches[i]);
        std::string s = decode_frame_line(e.ethertype, e.src_uid, e.dst_uid,
                                          e.payload.data(), e.payload.size(),
                                          e.wall_ms, opts);
        if ((int)s.size() > cols) s.resize(cols);
        new_lines[slot++] = std::move(s);
    }

    // Status bar.
    std::ostringstream bar;
    bar << (st.paused ? "PAUSED" : "LIVE  ")
        << "  filter: "
        << (st.filter.empty() ? std::string("(none)") : args.filter_expr);
    bar << "  shown: " << matches.size() << "/" << ring.size();
    if (st.paused && st.paused_dropped > 0) {
        bar << "  pending: " << st.paused_dropped;
    }
    bool audio_in_filter = st.filter.accepts_ethertype(0);
    if (!audio_in_filter && st.audio_seen > 0) {
        uint64_t span_ms = wall_now_ms() - st.audio_first_ms;
        double rate = span_ms > 0 ? (1000.0 * st.audio_seen / span_ms) : 0.0;
        bar << "  audio-suppressed: " << (int)rate << "/s";
    }
    std::string bar_s = bar.str();
    if ((int)bar_s.size() > cols) bar_s.resize(cols);

    // Initial paint or resize: clear once, write all body slots fresh, and
    // paint the static key-help footer. Key help only redrawn here because
    // the string is constant; tickly diffing it every refresh is wasted.
    if (st.needs_full_redraw) {
        clear_screen();
        st.prev_lines.assign(body_rows, std::string(1, '\x01'));  // sentinel != empty
        st.prev_status.clear();

        std::string help = KEY_HELP;
        if ((int)help.size() > cols) help.resize(cols);
        move_cursor(rows - 1, 1);
        std::cout << "\033[2m" << help << "\033[0m";  // dim
        clear_line_to_end();

        st.needs_full_redraw = false;
    }

    // Resize prev_lines to current body height (terminal may have shrunk
    // since last full redraw was skipped).
    if ((int)st.prev_lines.size() != body_rows) {
        st.prev_lines.resize(body_rows);
    }

    // Diff body rows: only redraw rows whose content actually changed.
    for (int r = 0; r < body_rows; ++r) {
        const std::string& want = new_lines[r];
        if (st.prev_lines[r] == want) continue;
        move_cursor(r + 1, 1);
        std::cout << want;
        clear_line_to_end();
        st.prev_lines[r] = want;
    }

    // Diff status bar.
    if (st.prev_status != bar_s) {
        move_cursor(rows, 1);
        std::cout << "\033[7m" << bar_s;
        // Pad inverse to full width.
        for (int i = (int)bar_s.size(); i < cols; ++i) std::cout << ' ';
        std::cout << "\033[0m";
        st.prev_status = bar_s;
    }

    st.prev_rows = rows;
    st.prev_cols = cols;

    std::cout << std::flush;
}

// Prompt for a new filter expression at the bottom row. Blocking read with
// canonical termios so backspace + Enter work normally; restore raw on exit.
std::string prompt_filter(const std::string& prefill) {
    int rows = term_rows();
    move_cursor(rows, 1);
    std::cout << "\033[7m/" << "\033[0m " << std::flush;

    // Temporarily restore cooked mode for line edit.
    if (g_termios_saved) ::tcsetattr(STDIN_FILENO, TCSANOW, &g_orig_termios);

    std::string s = prefill;
    // We re-echo the prefill ourselves so the user can edit it. Simplest:
    // read a single line via std::getline, ignoring prefill — the user
    // re-types from scratch. (Saves wiring up a real line editor.)
    (void)s;
    std::string in;
    std::getline(std::cin, in);

    // Back to raw.
    if (g_termios_saved) {
        struct termios t = g_orig_termios;
        t.c_lflag &= ~(ICANON | ECHO);
        t.c_cc[VMIN]  = 0;
        t.c_cc[VTIME] = 0;
        ::tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
    return in;
}

// Pcap I/O — own format. Header magic + version, then records.
class PcapWriter {
public:
    bool open(const std::string& path) {
        m_fp = std::fopen(path.c_str(), "wb");
        if (!m_fp) {
            std::cerr << "oaninspect: cannot open " << path
                      << ": " << ::strerror(errno) << "\n";
            return false;
        }
        std::fwrite(OSTPCAP_MAGIC, 1, sizeof(OSTPCAP_MAGIC), m_fp);
        std::fputc(OSTPCAP_VERSION, m_fp);
        return true;
    }
    void write(uint64_t ts_ms, const SimFrame& hdr, const uint8_t* payload) {
        if (!m_fp) return;
        std::fwrite(&ts_ms, sizeof(ts_ms), 1, m_fp);
        std::fwrite(&hdr, sizeof(hdr), 1, m_fp);
        std::fwrite(payload, 1, hdr.payload_len, m_fp);
    }
    ~PcapWriter() { if (m_fp) std::fclose(m_fp); }
private:
    FILE* m_fp{nullptr};
};

class PcapReader {
public:
    bool open(const std::string& path) {
        m_fp = std::fopen(path.c_str(), "rb");
        if (!m_fp) {
            std::cerr << "oaninspect: cannot open " << path
                      << ": " << ::strerror(errno) << "\n";
            return false;
        }
        char magic[sizeof(OSTPCAP_MAGIC)];
        if (std::fread(magic, 1, sizeof(magic), m_fp) != sizeof(magic)
            || std::memcmp(magic, OSTPCAP_MAGIC, sizeof(magic)) != 0) {
            std::cerr << "oaninspect: " << path << " is not an OSTPCAP file\n";
            return false;
        }
        int v = std::fgetc(m_fp);
        if (v != OSTPCAP_VERSION) {
            std::cerr << "oaninspect: OSTPCAP version mismatch (file=" << v
                      << " expected=" << (int)OSTPCAP_VERSION << ")\n";
            return false;
        }
        return true;
    }
    // Reads next record. Returns false on EOF or error.
    bool next(uint64_t& ts_ms, SimFrame& hdr, std::vector<uint8_t>& payload) {
        if (!m_fp) return false;
        if (std::fread(&ts_ms, sizeof(ts_ms), 1, m_fp) != 1) return false;
        if (std::fread(&hdr,   sizeof(hdr),   1, m_fp) != 1) return false;
        if (hdr.payload_len > MAX_FRAME_PAYLOAD) return false;
        payload.resize(hdr.payload_len);
        if (std::fread(payload.data(), 1, hdr.payload_len, m_fp) != hdr.payload_len) return false;
        return true;
    }
    ~PcapReader() { if (m_fp) std::fclose(m_fp); }
private:
    FILE* m_fp{nullptr};
};

// Stream-parser state for incoming SimFrames over the socket. Tolerates
// partial reads.
struct WireParser {
    bool                 have_hdr{false};
    SimFrame             hdr{};
    size_t               hdr_read{0};
    std::vector<uint8_t> payload;
    size_t               payload_read{0};

    // Drains bytes from `data` into the current frame. For each complete
    // frame, invokes `on_complete(hdr, payload)`. Returns true on success;
    // false if a malformed frame was seen (caller should disconnect).
    template <typename F>
    bool feed(const uint8_t* data, size_t n, F&& on_complete) {
        size_t i = 0;
        while (i < n) {
            if (!have_hdr) {
                size_t need = sizeof(SimFrame) - hdr_read;
                size_t take = std::min(need, n - i);
                std::memcpy(reinterpret_cast<uint8_t*>(&hdr) + hdr_read, data + i, take);
                hdr_read += take;
                i += take;
                if (hdr_read < sizeof(SimFrame)) return true;
                if (hdr.payload_len > MAX_FRAME_PAYLOAD) return false;
                payload.resize(hdr.payload_len);
                payload_read = 0;
                have_hdr = true;
            }
            if (have_hdr) {
                size_t need = hdr.payload_len - payload_read;
                size_t take = std::min(need, n - i);
                if (take > 0) std::memcpy(payload.data() + payload_read, data + i, take);
                payload_read += take;
                i += take;
                if (payload_read < hdr.payload_len) return true;
                on_complete(hdr, payload);
                have_hdr = false;
                hdr_read = 0;
                payload_read = 0;
            }
        }
        return true;
    }
};

} // namespace

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);

    struct sigaction sa{};
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    ::sigaction(SIGINT,  &sa, nullptr);
    ::sigaction(SIGTERM, &sa, nullptr);
    ::signal(SIGPIPE, SIG_IGN);

    UiState st;
    st.hex = args.hex;
    st.color = ::isatty(STDOUT_FILENO);

    // Default filter behaviour: if user didn't pass --filter, suppress
    // audio (it dominates the wire). Explicit empty --filter '' = wide open.
    std::string effective_filter = args.filter_expr;
    if (!args.filter_explicit) {
        effective_filter = "ethertype=disco,control,sync";
    }
    {
        std::string err;
        if (!st.filter.parse(effective_filter, err)) {
            std::cerr << "oaninspect: filter parse: " << err << "\n";
            return 2;
        }
    }
    // Stash the displayed filter expression for the status bar.
    args.filter_expr = effective_filter;

    Ring ring(args.buffer);

    PcapWriter pw;
    if (!args.pcap_path.empty() && !pw.open(args.pcap_path)) return 1;

    // The on-frame path is shared between live recv and replay so pause/
    // filter/scroll work identically.
    //
    // Pause semantics: while paused, frames keep arriving from the wire
    // but are *dropped*, not buffered. This freezes the displayed history
    // exactly where the user paused — otherwise the ring would silently
    // evict the very entries the user paused to read. `paused_dropped`
    // tracks how many frames got skipped so the status bar can show it.
    // Pcap recording, however, continues regardless — recording a session
    // to replay later is independent of what you're looking at right now.
    //
    // Audio fast-path: even at 100k ring capacity, 4000 audio frames/s
    // would push every disco/sync/control entry out within ~25 seconds. So
    // if the current filter excludes audio, we skip audio at ingest — the
    // ring only sees frames the user actually wants. Changing the filter
    // later to *include* audio means past audio is gone (as discussed —
    // acceptable trade), but everything else (sync, disco, control)
    // remains available for retroactive filtering while paused.
    // The `audio_seen` counter bumps regardless so the suppression hint
    // in the status bar still reflects reality.
    auto on_frame = [&](const SimFrame& hdr, const std::vector<uint8_t>& payload,
                        uint64_t wall_ms) {
        if (!args.pcap_path.empty()) pw.write(wall_ms, hdr, payload.data());

        uint8_t idx = etype_index_of(hdr.ethertype);
        if (idx == 0) {
            if (st.audio_seen == 0) st.audio_first_ms = wall_ms;
            st.audio_seen++;
            // Skip the push if the current filter excludes audio by
            // ethertype. (src/dst clauses are intentionally NOT consulted
            // here — even if the user has `src=42`, if they didn't
            // explicitly exclude audio we keep buffering it so a future
            // filter change that targets it has something to show.)
            if (!st.filter.accepts_ethertype(0)) return;
        }

        if (st.paused) {
            st.paused_dropped++;
            return;
        }

        Entry e;
        e.wall_ms   = wall_ms;
        e.ethertype = hdr.ethertype;
        e.src_uid   = hdr.src_uid;
        e.dst_uid   = hdr.dest_uid;
        e.etype_idx = idx;
        e.payload   = payload;
        ring.push(std::move(e));
    };

    enable_raw_stdin();
    bool stdin_is_tty = ::isatty(STDIN_FILENO);

    // Replay path: just stream the file through on_frame and render once,
    // then drop into the input loop for browsing.
    if (!args.replay_path.empty()) {
        PcapReader pr;
        if (!pr.open(args.replay_path)) return 1;
        uint64_t ts; SimFrame hdr; std::vector<uint8_t> payload;
        while (pr.next(ts, hdr, payload)) on_frame(hdr, payload, ts);
        st.paused = true;  // replay defaults to paused so user can scroll
        st.scroll_offset = 0;

        // Non-TTY (replay piped or stdin closed): there's nothing to
        // interact with, so dump every matching frame to stdout once and
        // exit. Useful for scripting / sanity-checks of a recorded file.
        if (!stdin_is_tty) {
            DecodeOpts opts{st.hex, false};
            for (size_t i = 0; i < ring.size(); ++i) {
                const auto& e = ring.at(i);
                if (!st.filter.match(e.etype_idx, e.src_uid, e.dst_uid)) continue;
                std::cout << decode_frame_line(e.ethertype, e.src_uid, e.dst_uid,
                                               e.payload.data(), e.payload.size(),
                                               e.wall_ms, opts) << "\n";
            }
            return 0;
        }

        render(ring, st, args);

        while (!g_shutdown_requested.load()) {
            pollfd p{STDIN_FILENO, POLLIN, 0};
            int pr_rc = ::poll(&p, 1, 200);
            if (pr_rc < 0) {
                if (errno == EINTR) continue;
                break;
            }
            if (pr_rc == 0) { render(ring, st, args); continue; }
            char c;
            if (::read(STDIN_FILENO, &c, 1) != 1) continue;

            bool quit = false;
            int body_rows = std::max(1, term_rows() - 1);
            switch (c) {
                case 'q': case 3 /*Ctrl-C*/: quit = true; break;
                case ' ':
                    st.paused = !st.paused;
                    if (!st.paused) st.paused_dropped = 0;
                    break;
                case 'j': st.scroll_offset = std::max(0, st.scroll_offset - 1); break;
                case 'k': st.scroll_offset = std::min((int)ring.size(), st.scroll_offset + 1); break;
                case 4 /*Ctrl-D*/: st.scroll_offset = std::max(0, st.scroll_offset - body_rows/2); break;
                case 21/*Ctrl-U*/: st.scroll_offset = std::min((int)ring.size(), st.scroll_offset + body_rows/2); break;
                case 'g': st.scroll_offset = (int)ring.size(); break;
                case 'G': st.scroll_offset = 0; break;
                case 'h': st.hex = !st.hex; break;
                case '/': {
                    std::string s = prompt_filter(args.filter_expr);
                    std::string err;
                    Filter f;
                    if (f.parse(s, err)) {
                        st.filter = std::move(f);
                        args.filter_expr = s;
                    } else {
                        // Re-render then show error line briefly. Simplest:
                        // squash it into the filter expression for visibility.
                        args.filter_expr = "(parse error: " + err + ")";
                    }
                    st.scroll_offset = 0;
                    // prompt_filter wrote to the bottom row; force a clean
                    // repaint so the prompt artifact doesn't linger.
                    st.needs_full_redraw = true;
                    break;
                }
                default: break;
            }
            if (quit) break;
            render(ring, st, args);
        }
        return 0;
    }

    // Live path: socket + stdin.
    int sock = open_socket_or_die(args.socket_path);
    WireParser parser;
    std::vector<uint8_t> rx_buf(4096);

    uint64_t last_render_ms = 0;
    bool stdin_open = stdin_is_tty;  // only poll stdin if it's a real tty

    // Non-tty live path (stdin closed / piped): act like a plain ANSI tail —
    // emit a line per matching frame to stdout, no curses, no pause. Used by
    // scripts/CI to capture decoded traffic.
    if (!stdin_is_tty) {
        DecodeOpts opts{st.hex, false};
        pollfd pfds[1] = { { sock, POLLIN, 0 } };
        while (!g_shutdown_requested.load()) {
            int rc = ::poll(pfds, 1, 200);
            if (rc < 0) { if (errno == EINTR) continue; break; }
            if (rc == 0) continue;
            if (!(pfds[0].revents & POLLIN)) {
                // hangup / error
                break;
            }
            ssize_t n = ::read(sock, rx_buf.data(), rx_buf.size());
            if (n <= 0) {
                if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
                if (n < 0 && errno == EINTR) continue;
                break;
            }
            bool ok = parser.feed(rx_buf.data(), n,
                [&](const SimFrame& hdr, const std::vector<uint8_t>& p) {
                    on_frame(hdr, p, wall_now_ms());
                    uint8_t idx = etype_index_of(hdr.ethertype);
                    if (!st.filter.match(idx, hdr.src_uid, hdr.dest_uid)) return;
                    std::cout << decode_frame_line(hdr.ethertype, hdr.src_uid,
                                                   hdr.dest_uid, p.data(), p.size(),
                                                   wall_now_ms(), opts) << "\n";
                });
            if (!ok) {
                std::cerr << "oaninspect: malformed frame, exiting\n";
                break;
            }
            std::cout.flush();
        }
        ::close(sock);
        return 0;
    }

    while (!g_shutdown_requested.load()) {
        pollfd pfds[2] = {
            { sock, POLLIN, 0 },
            { STDIN_FILENO, stdin_open ? (short)POLLIN : (short)0, 0 }
        };
        int rc = ::poll(pfds, stdin_open ? 2 : 1, 200);
        if (rc < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (rc > 0) {
            if (pfds[0].revents & POLLIN) {
                while (true) {
                    ssize_t n = ::read(sock, rx_buf.data(), rx_buf.size());
                    if (n > 0) {
                        bool ok = parser.feed(rx_buf.data(), n,
                            [&](const SimFrame& hdr, const std::vector<uint8_t>& p) {
                                on_frame(hdr, p, wall_now_ms());
                            });
                        if (!ok) {
                            std::cerr << "\noaninspect: malformed frame, exiting\n";
                            g_shutdown_requested = true;
                            break;
                        }
                        continue;
                    }
                    if (n == 0) {
                        // Daemon closed. Render once with whatever we have
                        // and bail.
                        std::cerr << "\noaninspect: daemon closed connection\n";
                        g_shutdown_requested = true;
                        break;
                    }
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    if (errno == EINTR) continue;
                    g_shutdown_requested = true;
                    break;
                }
            }
            if (stdin_open && (pfds[1].revents & (POLLIN|POLLHUP))) {
                char c;
                ssize_t rn;
                bool got_any = false;
                while ((rn = ::read(STDIN_FILENO, &c, 1)) == 1) {
                    got_any = true;
                    bool quit = false;
                    int body_rows = std::max(1, term_rows() - 1);
                    switch (c) {
                        case 'q': case 3: quit = true; break;
                        case ' ':
                    st.paused = !st.paused;
                    if (!st.paused) st.paused_dropped = 0;
                    break;
                        case 'j': st.scroll_offset = std::max(0, st.scroll_offset - 1); break;
                        case 'k': st.scroll_offset = std::min((int)ring.size(), st.scroll_offset + 1); break;
                        case 4: st.scroll_offset = std::max(0, st.scroll_offset - body_rows/2); break;
                        case 21: st.scroll_offset = std::min((int)ring.size(), st.scroll_offset + body_rows/2); break;
                        case 'g': st.scroll_offset = (int)ring.size(); break;
                        case 'G': st.scroll_offset = 0; break;
                        case 'h': st.hex = !st.hex; break;
                        case '/': {
                            std::string s = prompt_filter(args.filter_expr);
                            std::string err;
                            Filter f;
                            if (f.parse(s, err)) {
                                st.filter = std::move(f);
                                args.filter_expr = s;
                            } else {
                                args.filter_expr = "(parse error: " + err + ")";
                            }
                            st.scroll_offset = 0;
                            break;
                        }
                        default: break;
                    }
                    if (quit) { g_shutdown_requested = true; break; }
                }
                // If read returned 0 (EOF) and we got no bytes this round,
                // stdin has closed (e.g. terminal hung up). Stop polling it.
                if (rn == 0 && !got_any) stdin_open = false;
            }
        }

        // Throttle redraw to ~5Hz so a busy wire (high audio rate, filter
        // off) doesn't melt the terminal.
        uint64_t t = now_ms();
        if (t - last_render_ms >= 200 || rc > 0) {
            if (!st.paused) st.scroll_offset = 0;
            render(ring, st, args);
            last_render_ms = t;
        }
    }

    // Cursor + screen cleanup. Leave a final newline so the shell prompt
    // doesn't land mid-status-bar.
    move_cursor(term_rows(), 1);
    std::cout << "\033[0m\n" << std::flush;
    ::close(sock);
    return 0;
}
