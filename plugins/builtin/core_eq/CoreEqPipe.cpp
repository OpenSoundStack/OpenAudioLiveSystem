// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreEqPipe.h"

CoreEqPipe::CoreEqPipe() : m_peaks{
    PeakFilter{0, 0, 0, 96000.0f},
    PeakFilter{0, 0, 0, 96000.0f},
    PeakFilter{0, 0, 0, 96000.0f},
    PeakFilter{0, 0, 0, 96000.0f},
    PeakFilter{0, 0, 0, 96000.0f},
    PeakFilter{0, 0, 0, 96000.0f},
} {
    init_filters();
}

float CoreEqPipe::process_sample(float sample) {
    float last_sample = sample;

    for (int i = 0; i < 6; i++) {
        if (m_peaks[i].get_gain() != 0.0f) {
            last_sample = m_peaks[i].push_sample(last_sample);
        }
    }

    return last_sample;
}

void CoreEqPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id >= 1 && pck.packet_data.control_id <= 6) {
        FilterParams pfd{};
        memcpy(&pfd, pck.packet_data.data, sizeof(FilterParams));

        auto& filter = m_peaks[pck.packet_data.control_id - 1];
        filter.set_cutoff(pfd.fc);
        filter.set_gain(pfd.gain);
        filter.set_quality_factor(pfd.Q);
    }
}

void CoreEqPipe::init_filters() {
    for (int i = 0; i < 6; i++) {
        m_peaks[i] = PeakFilter(default_frequencies[i], default_Q, 0, 96000.0f);
    }
}
