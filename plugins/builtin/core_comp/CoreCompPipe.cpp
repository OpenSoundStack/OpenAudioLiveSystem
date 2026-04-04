// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompPipe.h"

CoreCompPipe::CoreCompPipe() {
    m_dynproc = std::make_unique<Dynamics>(
        [this](float level_lin) { return transfer_function(level_lin); },
        m_attack_ms,
        m_release_ms,
        m_hold_ms,
        96000
    );

    m_threshold_db = 0.0f;
    m_ratio_db = 0.0f;
    m_makeup_gain_lin = 1.0f;

    m_attack_ms = 10;
    m_release_ms = 120;
    m_hold_ms = 20;
}

float CoreCompPipe::transfer_function(float level_db) const {
    if (level_db > m_threshold_db) {
        return -(level_db - m_threshold_db) * m_ratio_db;
    } else {
        return 0.0f;
    }
}

float CoreCompPipe::process_sample(float sample) {
    float gain = m_dynproc->push_sample(sample) * m_makeup_gain_lin;
    return sample * gain;
}

void CoreCompPipe::apply_control(ControlPacket &pck) {

}
