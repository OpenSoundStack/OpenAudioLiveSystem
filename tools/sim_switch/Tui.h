#ifndef OSST_SIM_SWITCH_TUI_H
#define OSST_SIM_SWITCH_TUI_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class Switch;

// Exponentially-weighted moving average — smooths the wildly bursty
// instantaneous rate readings (disco at 5-sec intervals would otherwise
// oscillate between 20/s and 0/s, etc.). α=0.15 gives a ~1-second effective
// window at the 200ms TUI refresh cadence.
struct Ewma {
    double smoothed{0.0};
    bool   seeded{false};
    void update(double instant, double alpha = 0.15) {
        if (!seeded) { smoothed = instant; seeded = true; return; }
        smoothed = alpha * instant + (1.0 - alpha) * smoothed;
    }
};

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

    // Per-peer tx/rx snapshots so we can compute deltas frame-over-frame
    // without making the Switch keep a second copy.
    struct PeerSnapshot {
        uint64_t tx[5]{};
        uint64_t rx[5]{};
        Ewma     tx_rate[5]{};
        Ewma     rx_rate[5]{};
    };

    std::string m_socket_path;
    bool        m_headless;

    // Sampled at the previous refresh — used to compute per-second rates.
    uint64_t m_prev_frames[5]{};
    uint64_t m_prev_bytes[5]{};
    uint64_t m_prev_bcast[5]{};
    uint64_t m_prev_now_ms{0};

    // Smoothed totals (per EtherType).
    Ewma m_frame_rate[5]{};
    Ewma m_byte_rate[5]{};

    // Per-peer snapshots keyed by uid. Pruned when the switch drops them.
    std::unordered_map<uint16_t, PeerSnapshot> m_peer_snaps;

    // For non-flicker redraw — track the previous render's line count.
    int m_lines_rendered{0};

    // For headless mode — emit only every 5s.
    uint64_t m_last_headless_ms{0};
};

#endif
