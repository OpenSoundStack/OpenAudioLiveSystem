// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompPipe.h"

CoreCompPipe::CoreCompPipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper) {
    m_nmapper = std::move(nmapper);
    m_router = router;

    m_threshold_db = CompDefaultParams::static_defaults.threshold;
    m_ratio_db = 1.0f - (1.0f / CompDefaultParams::static_defaults.ratio);
    m_makeup_gain_lin = CompDefaultParams::static_defaults.gain;

    m_attack_ms = CompDefaultParams::dyn_defaults.attack_ms;
    m_release_ms = CompDefaultParams::dyn_defaults.release_ms;
    m_hold_ms = CompDefaultParams::dyn_defaults.hold_ms;

    m_reduction_send_counter = 0;
    m_current_enveloppe = 0.0f;

    m_dynproc = std::make_unique<Dynamics>(
        [this](float level_lin) { return transfer_function(level_lin); },
        m_attack_ms,
        m_release_ms,
        m_hold_ms,
        96000
    );
}

float CoreCompPipe::transfer_function(float level_db) {
    m_current_enveloppe = level_db;

    if (level_db > m_threshold_db) {
        float gain = -level_db * m_ratio_db + m_threshold_db * m_ratio_db;

        return gain;
    } else {
        return 0.0f;
    }
}

float CoreCompPipe::process_sample(float sample) {
    float gain = m_dynproc->push_sample(sample) * m_makeup_gain_lin;

    send_feedback(gain, m_current_enveloppe);

    return sample * gain;
}

void CoreCompPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        auto* params = reinterpret_cast<CompStaticParams*>(pck.packet_data.data);

        m_threshold_db = params->threshold;
        m_ratio_db = 1.0f - (1.0f / params->ratio);
        m_makeup_gain_lin = params->gain;
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

void CoreCompPipe::send_feedback(float gain_lin, float enveloppe_db) {
    if (m_reduction_send_counter == 0) {
        ControlPacket pck{};
        pck.header.type = PacketType::CONTROL;
        pck.packet_data.channel = get_channel();
        pck.packet_data.control_id = 1;
        pck.packet_data.control_type = DataTypes::FLOAT;
        pck.packet_data.elem_index = get_index();
        memcpy(&pck.packet_data.data[0], &gain_lin, sizeof(float));
        memcpy(&pck.packet_data.data[1], &enveloppe_db, sizeof(float));

        auto surfaces = m_nmapper->find_all_control_surfaces();

        for (auto& surface : surfaces) {
            m_router->send_control_packet(pck, surface);
        }
    }

    m_reduction_send_counter = (m_reduction_send_counter + 1) % 1000;
}
