// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#include "AudioInPipe.h"

AudioInPipe::AudioInPipe(AudioRouter* router) {
    m_router = router;

    m_in_gain = 1; // 0dB gain by default
    m_in_trim = 1;

    construct_hw_packet(0);
}

float AudioInPipe::process_sample(float sample) {
    return (sample * m_in_trim);
}

void AudioInPipe::set_gain_lin(float gain) {
    m_in_gain = gain;

    memcpy(&m_hw_control.packet_data.data, &m_in_gain, sizeof(float));
    m_router->send_control_packet(m_hw_control, 2);
}

void AudioInPipe::set_trim_lin(float trim) {
    m_in_trim = trim;
}

void AudioInPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        GainTrim gt{};
        memcpy(&gt, &pck.packet_data.data, sizeof(GainTrim));

        float new_trim = gt.trim;
        float new_gain = gt.gain;

        set_gain_lin(new_gain);
        set_trim_lin(new_trim);
    }
}

void AudioInPipe::construct_hw_packet(uint8_t channel) {
    m_hw_control.header.type = PacketType::CONTROL;
    m_hw_control.packet_data.channel = channel;
    m_hw_control.packet_data.control_id = 0;
    m_hw_control.packet_data.control_type = DataTypes::FLOAT;
    m_hw_control.packet_data.elem_index = 0; // Unused for hardware control
}
