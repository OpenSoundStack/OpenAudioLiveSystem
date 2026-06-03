#include "Tui.h"
#include "Switch.h"

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

    if (m_headless) {
        if (now_ms - m_last_headless_ms >= 5000) {
            render_headless(sw, dt_s, now_ms);
            m_last_headless_ms = now_ms;
            // Snapshot for next dt calc:
            m_prev_now_ms = now_ms;
            for (int i = 0; i < 5; ++i) {
                m_prev_frames[i] = sw.stats().frames_in[i];
                m_prev_bytes[i]  = sw.stats().bytes_in[i];
                m_prev_bcast[i]  = sw.stats().bcast_in[i];
            }
        }
    } else {
        render_tui(sw, dt_s);
        m_prev_now_ms = now_ms;
        for (int i = 0; i < 5; ++i) {
            m_prev_frames[i] = sw.stats().frames_in[i];
            m_prev_bytes[i]  = sw.stats().bytes_in[i];
            m_prev_bcast[i]  = sw.stats().bcast_in[i];
        }
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

void Tui::render_tui(const Switch& sw, double dt_s) {
    erase_previous();

    std::vector<std::string> lines;

    auto conns = sw.conns();

    {
        std::ostringstream l;
        l << "sim_switch  " << m_socket_path
          << "   conns=" << conns.size();
        lines.push_back(l.str());
    }
    lines.push_back("");
    lines.push_back("Traffic (msg/s / KiB/s / bcast%):");

    for (int i = 0; i < 5; ++i) {
        uint64_t df = sw.stats().frames_in[i] - m_prev_frames[i];
        uint64_t db = sw.stats().bytes_in[i]  - m_prev_bytes[i];
        uint64_t dc = sw.stats().bcast_in[i]  - m_prev_bcast[i];

        if (i == 4 && df == 0 && sw.stats().frames_in[i] == 0) continue;

        double fps = df / dt_s;
        double kbps = (db / dt_s) / 1024.0;
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
            std::ostringstream l;
            l << "  fd=" << std::setw(3) << c.fd
              << "  " << (c.hello_received ? "OK   " : "PRE  ")
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

void Tui::render_headless(const Switch& sw, double dt_s, uint64_t now_ms) {
    char ts[16];
    time_t sec = now_ms / 1000;
    struct tm tm_local;
    localtime_r(&sec, &tm_local);
    std::strftime(ts, sizeof(ts), "%H:%M:%S", &tm_local);

    auto conns = sw.conns();
    uint64_t total_drops = 0;
    for (const auto& c : conns) total_drops += c.drops;

    double fps[5];
    for (int i = 0; i < 5; ++i) {
        uint64_t df = sw.stats().frames_in[i] - m_prev_frames[i];
        fps[i] = df / dt_s;
    }

    std::cout << "[" << ts << "]"
              << " conns=" << conns.size()
              << " audio=" << std::fixed << std::setprecision(1) << fps[0] << "/s"
              << " disco=" << fps[1] << "/s"
              << " control=" << fps[2] << "/s"
              << " sync=" << fps[3] << "/s"
              << " drops=" << total_drops
              << "\n" << std::flush;
}
