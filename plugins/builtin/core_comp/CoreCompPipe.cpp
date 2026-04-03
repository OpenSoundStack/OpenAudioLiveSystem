// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

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
