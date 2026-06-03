#include "Tui.h"
#include "Switch.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace {

const char* device_type_name(DeviceType t) {
    switch (t) {
        case DeviceType::CONTROL_SURFACE:    return "CONTROL_SURFACE";
        case DeviceType::MONITORING:         return "MONITORING";
        case DeviceType::AUDIO_IO_INTERFACE: return "AUDIO_IO";
        case DeviceType::AUDIO_DSP:          return "AUDIO_DSP";
    }
    return "?";
}

std::string mac_to_string(uint64_t mac_u64) {
    uint8_t b[8];
    std::memcpy(b, &mac_u64, 8);
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
                  b[0], b[1], b[2], b[3], b[4], b[5]);
    return buf;
}

const char* etype_label(int idx) {
    switch (idx) {
        case 0: return "audio  ";
        case 1: return "disco  ";
        case 2: return "control";
        case 3: return "sync   ";
        case 4: return "other  ";
    }
    return "?      ";
}

uint16_t etype_value(int idx) {
    switch (idx) {
        case 0: return 0x0681;
        case 1: return 0x0682;
        case 2: return 0x0683;
        case 3: return 0x0684;
        default: return 0;
    }
}

// Compact "tx/rx" pair for the Peers row. Pads to a consistent width so
// columns line up across rows.
std::string fmt_pair(double tx, double rx) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%6.1f/%-6.1f", tx, rx);
    return buf;
}

} // namespace

namespace {
// Restores cursor visibility on any normal process exit (atexit covers
// std::exit, returning from main, uncaught exception std::terminate paths).
// SIGKILL/SIGSEGV bypass atexit by design — nothing we can do there.
void restore_cursor() {
    // Use raw write() to stdout — std::cout may already have been torn down
    // by static destructors running before atexit handlers on some toolchains.
    const char esc[] = "\033[?25h";
    ssize_t r = ::write(STDOUT_FILENO, esc, sizeof(esc) - 1);
    (void)r;
}
} // namespace

Tui::Tui(std::string socket_path, bool headless)
    : m_socket_path(std::move(socket_path)), m_headless(headless) {
    // Auto-fall-back to headless when not on a tty (background, pipe, file).
    if (!m_headless && !::isatty(STDOUT_FILENO)) m_headless = true;
    if (!m_headless) {
        std::cout << "\033[?25l" << std::flush;  // hide cursor
        // Defensive: even if shutdown() never runs (uncaught signal handler,
        // std::terminate, etc.), restore the cursor before the process dies.
        static bool s_atexit_installed = false;
        if (!s_atexit_installed) {
            std::atexit(restore_cursor);
            s_atexit_installed = true;
        }
    }
}

void Tui::shutdown() {
    if (!m_headless) {
        std::cout << "\033[?25h" << std::flush;  // show cursor
    }
}

void Tui::refresh(const Switch& sw, uint64_t now_ms) {
    if (m_prev_now_ms == 0) {
        m_prev_now_ms = now_ms;
        for (int i = 0; i < 5; ++i) {
            m_prev_frames[i] = sw.stats().frames_in[i];
            m_prev_bytes[i]  = sw.stats().bytes_in[i];
            m_prev_bcast[i]  = sw.stats().bcast_in[i];
        }
        // First call: don't render rates (no dt yet). Show structure only.
        if (!m_headless) render_tui(sw, 1.0);
        return;
    }

    double dt_s = (now_ms - m_prev_now_ms) / 1000.0;
    if (dt_s <= 0) dt_s = 0.001;

    // Update EWMA rates from instantaneous deltas. Done in refresh() so it
    // happens regardless of headless vs. TUI rendering — the headless path
    // wants smoothed values too.
    for (int i = 0; i < 5; ++i) {
        double inst_fps = (sw.stats().frames_in[i] - m_prev_frames[i]) / dt_s;
        double inst_bps = (sw.stats().bytes_in[i]  - m_prev_bytes[i])  / dt_s;
        m_frame_rate[i].update(inst_fps);
        m_byte_rate[i].update(inst_bps);
    }
    for (const auto& [uid, ps] : sw.peer_stats()) {
        auto& snap = m_peer_snaps[uid];
        for (int i = 0; i < 5; ++i) {
            double inst_tx = (ps.tx[i] - snap.tx[i]) / dt_s;
            double inst_rx = (ps.rx[i] - snap.rx[i]) / dt_s;
            snap.tx_rate[i].update(inst_tx);
            snap.rx_rate[i].update(inst_rx);
            snap.tx[i] = ps.tx[i];
            snap.rx[i] = ps.rx[i];
        }
    }
    // Drop snapshots for peers the switch has pruned, so the Peers table
    // doesn't keep showing dead uids.
    for (auto it = m_peer_snaps.begin(); it != m_peer_snaps.end(); ) {
        if (sw.peer_stats().find(it->first) == sw.peer_stats().end()) {
            it = m_peer_snaps.erase(it);
        } else {
            ++it;
        }
    }

    if (m_headless) {
        if (now_ms - m_last_headless_ms >= 5000) {
            render_headless(sw, dt_s, now_ms);
            m_last_headless_ms = now_ms;
        }
    } else {
        render_tui(sw, dt_s);
    }

    m_prev_now_ms = now_ms;
    for (int i = 0; i < 5; ++i) {
        m_prev_frames[i] = sw.stats().frames_in[i];
        m_prev_bytes[i]  = sw.stats().bytes_in[i];
        m_prev_bcast[i]  = sw.stats().bcast_in[i];
    }
}

void Tui::erase_previous() {
    if (m_lines_rendered <= 0) return;
    // Cursor up N lines.
    std::cout << "\033[" << m_lines_rendered << "A";
    // Each line: clear and re-print.
    // We don't actually erase yet — emit_line does \033[2K\r at each new line.
}

void Tui::emit_line(const std::string& line) {
    // \033[2K = erase entire line; \r = carriage return so we start at col 0.
    std::cout << "\033[2K\r" << line << "\n";
}

void Tui::render_tui(const Switch& sw, double /*dt_s*/) {
    erase_previous();

    std::vector<std::string> lines;

    auto conns = sw.conns();
    int inspectors = sw.inspector_count();

    {
        std::ostringstream l;
        l << "sim_switch  " << m_socket_path
          << "   conns=" << conns.size()
          << "   inspectors=" << inspectors;
        lines.push_back(l.str());
    }
    lines.push_back("");
    lines.push_back("Traffic (msg/s / KiB/s / bcast%, smoothed):");

    for (int i = 0; i < 5; ++i) {
        uint64_t df = sw.stats().frames_in[i] - m_prev_frames[i];
        uint64_t dc = sw.stats().bcast_in[i]  - m_prev_bcast[i];

        if (i == 4 && m_frame_rate[i].smoothed < 0.05 && sw.stats().frames_in[i] == 0) continue;

        double fps = m_frame_rate[i].smoothed;
        double kbps = m_byte_rate[i].smoothed / 1024.0;
        double bcast_pct = df > 0 ? (100.0 * dc / df) : 0.0;

        std::ostringstream l;
        l << "  " << etype_label(i)
          << "  0x" << std::hex << std::setw(4) << std::setfill('0') << etype_value(i)
          << std::dec << std::setfill(' ')
          << "  " << std::fixed << std::setprecision(1) << std::setw(7) << fps << "/s"
          << "  " << std::fixed << std::setprecision(1) << std::setw(7) << kbps << " KiB/s"
          << "  " << std::fixed << std::setprecision(0) << std::setw(3) << bcast_pct << "%";
        lines.push_back(l.str());
    }

    // Peers table — built from peer_stats keyed by uid, joined to the
    // discovery table for the friendly device name.
    lines.push_back("");
    lines.push_back("Peers (msg/s — A/D/C/S = audio/disco/control/sync, tx/rx):");
    if (m_peer_snaps.empty()) {
        lines.push_back("  (no peers active)");
    } else {
        // Stable display: sort by uid. Hash-map order would flicker.
        std::vector<uint16_t> uids;
        uids.reserve(m_peer_snaps.size());
        for (const auto& [uid, _] : m_peer_snaps) uids.push_back(uid);
        std::sort(uids.begin(), uids.end());

        for (uint16_t uid : uids) {
            const auto& snap = m_peer_snaps[uid];
            // Look up the friendly name from discovery if we have it.
            std::string name = "(unknown)";
            auto it = sw.disco().devices().find(uid);
            if (it != sw.disco().devices().end() && !it->second.name.empty()) {
                name = it->second.name;
            }

            std::ostringstream l;
            l << "  uid=" << std::setw(5) << uid
              << "  " << std::left << std::setw(20) << name << std::right
              << "  A:" << fmt_pair(snap.tx_rate[0].smoothed, snap.rx_rate[0].smoothed)
              << "  D:" << fmt_pair(snap.tx_rate[1].smoothed, snap.rx_rate[1].smoothed)
              << "  C:" << fmt_pair(snap.tx_rate[2].smoothed, snap.rx_rate[2].smoothed)
              << "  S:" << fmt_pair(snap.tx_rate[3].smoothed, snap.rx_rate[3].smoothed);
            lines.push_back(l.str());
        }
    }

    lines.push_back("");
    lines.push_back("Devices:");
    if (sw.disco().devices().empty()) {
        lines.push_back("  (none discovered yet)");
    } else {
        for (const auto& [uid, d] : sw.disco().devices()) {
            std::ostringstream l;
            l << "  uid=" << std::setw(5) << uid
              << "  mac=" << mac_to_string(d.mac)
              << "  " << std::left << std::setw(20) << d.name << std::right
              << "  type=" << std::left << std::setw(16) << device_type_name(d.type) << std::right
              << "  in="    << (int)d.topo.phy_in_count
              << "  out="   << (int)d.topo.phy_out_count
              << "  pipes=" << (int)d.topo.pipes_count;
            lines.push_back(l.str());
        }
    }

    lines.push_back("");
    lines.push_back("Conns:");
    if (conns.empty()) {
        lines.push_back("  (none)");
    } else {
        for (const auto& c : conns) {
            const char* state =
                !c.hello_received ? "PRE  " :
                c.promiscuous     ? "INSP " :
                                    "OK   ";
            std::ostringstream l;
            l << "  fd=" << std::setw(3) << c.fd
              << "  " << state
              << "  etype=0x" << std::hex << std::setw(4) << std::setfill('0') << c.ethertype
              << std::dec << std::setfill(' ')
              << "  uid=" << std::setw(5) << c.self_uid
              << "  drops=" << c.drops;
            lines.push_back(l.str());
        }
    }

    int new_lines = static_cast<int>(lines.size());
    for (const auto& l : lines) emit_line(l);

    // If previous render had more lines than this one, blank the extras and
    // move the cursor back up over them so they get overwritten next time.
    int extra = m_lines_rendered - new_lines;
    for (int i = 0; i < extra; ++i) emit_line("");
    if (extra > 0) std::cout << "\033[" << extra << "A";

    std::cout << std::flush;
    m_lines_rendered = new_lines;
}

void Tui::render_headless(const Switch& sw, double /*dt_s*/, uint64_t now_ms) {
    char ts[16];
    time_t sec = now_ms / 1000;
    struct tm tm_local;
    localtime_r(&sec, &tm_local);
    std::strftime(ts, sizeof(ts), "%H:%M:%S", &tm_local);

    auto conns = sw.conns();
    uint64_t total_drops = 0;
    for (const auto& c : conns) total_drops += c.drops;

    std::cout << "[" << ts << "]"
              << " conns=" << conns.size()
              << " inspectors=" << sw.inspector_count()
              << " audio=" << std::fixed << std::setprecision(1) << m_frame_rate[0].smoothed << "/s"
              << " disco=" << m_frame_rate[1].smoothed << "/s"
              << " control=" << m_frame_rate[2].smoothed << "/s"
              << " sync=" << m_frame_rate[3].smoothed << "/s"
              << " drops=" << total_drops
              << "\n" << std::flush;
}
