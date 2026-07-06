// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "LevelMeasurePipe.h"

LevelMeasurePipe::LevelMeasurePipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper) {
    m_router = router;
    m_nmapper = nmapper;

    m_value_counter = 0;
    m_sum = 0.0f;

    // Initialize sample list
    // 4800 samples = 50 ms @ 96 kHz sampling rate
    for (int i = 0; i < 4800; i++) {
        m_rms_buffer.push(0);
    }
}


void LevelMeasurePipe::process_samples(std::span<float>& audio_data) {
    // Efficient RMS calculation
    // Add the latest sample and subtract the oldest
    // Avoids to read the ENTIRE 4.8k element list every sample...

    for (auto s : audio_data) {
        float sample_sat = s;
        if (std::abs(s) > 1.0f) {
            sample_sat = 1.0f;
        }

        float sample2 = sample_sat * sample_sat;
        m_sum += sample2;

        m_rms_buffer.push(sample2);

        m_sum -= m_rms_buffer.front();
        m_rms_buffer.pop();

        m_value_counter++;
        if (m_value_counter > 300) {
            float mean = m_sum / 28000.0f;
            float rms = std::sqrt(mean);
            float mean_db = 10 * std::log10(rms); // Max level is 1.0f

            feedback_send(mean_db);

            m_value_counter = 0;
        }
    }
}

void LevelMeasurePipe::feedback_send(float db_level) {
    ControlPacket pck{};
    pck.header.type = PacketType::CONTROL;
    pck.packet_data.channel = get_channel();
    pck.packet_data.control_id = 0;
    pck.packet_data.control_type = DataTypes::FLOAT;
    memcpy(pck.packet_data.data, &db_level, sizeof(float));

    auto surfaces = m_nmapper->find_all_control_surfaces();

    for (auto& surface : surfaces) {
        m_router->send_control_packet(pck, surface);
    }
}
