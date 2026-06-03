#ifndef OSST_SIM_SWITCH_TUI_H
#define OSST_SIM_SWITCH_TUI_H

#include <cstdint>
#include <string>
#include <vector>

class Switch;

class Tui {
public:
    explicit Tui(std::string socket_path, bool headless);

    // Called at ~5Hz from the main loop. Reads stats from the Switch (samples
    // rate over the elapsed wall-time since the last call) and renders.
    void refresh(const Switch& sw, uint64_t now_ms);

    // Final cleanup — restore cursor visibility, move to bottom.
    void shutdown();

private:
    void render_tui(const Switch& sw, double dt_s);
    void render_headless(const Switch& sw, double dt_s, uint64_t now_ms);

    void erase_previous();
    void emit_line(const std::string& line);

    std::string m_socket_path;
    bool        m_headless;

    // Sampled at the previous refresh — used to compute per-second rates.
    uint64_t m_prev_frames[5]{};
    uint64_t m_prev_bytes[5]{};
    uint64_t m_prev_bcast[5]{};
    uint64_t m_prev_now_ms{0};

    // For non-flicker redraw — track the previous render's line count.
    int m_lines_rendered{0};

    // For headless mode — emit only every 5s.
    uint64_t m_last_headless_ms{0};
};

#endif
