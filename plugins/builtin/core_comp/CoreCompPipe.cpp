// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompPipe.h"

CoreCompPipe::CoreCompPipe() {
    m_threshold_db = CompDefaultParams::static_defaults.threshold;
    m_ratio_db = CompDefaultParams::static_defaults.ratio;
    m_makeup_gain_lin = CompDefaultParams::static_defaults.gain;

    m_attack_ms = CompDefaultParams::dyn_defaults.attack_ms;
    m_release_ms = CompDefaultParams::dyn_defaults.release_ms;
    m_hold_ms = CompDefaultParams::dyn_defaults.hold_ms;

    m_dynproc = std::make_unique<Dynamics>(
        [this](float level_lin) { return transfer_function(level_lin); },
        m_attack_ms,
        m_release_ms,
        m_hold_ms,
        96000
    );
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
    if (pck.packet_data.control_id == 1) {
        auto* params = reinterpret_cast<CompStaticParams*>(pck.packet_data.data);

        m_threshold_db = params->threshold;
        m_ratio_db = params->ratio;
        m_makeup_gain_lin = std::pow(10.0f, params->gain / 10.0f);
    } else if (pck.packet_data.control_id == 2) {
        auto* params = reinterpret_cast<CompDynamicsParams*>(pck.packet_data.data);

        m_attack_ms = params->attack_ms;
        m_release_ms = params->release_ms;
        m_hold_ms = params->hold_ms;

        update_time_params();
    }
}

void CoreCompPipe::update_time_params() {
    m_dynproc->set_attack(m_attack_ms);
    m_dynproc->set_release(m_release_ms);
    m_dynproc->set_hold(m_hold_ms);
}
