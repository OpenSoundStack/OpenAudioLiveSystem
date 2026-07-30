// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "AudioInPipe.h"

AudioInPipe::AudioInPipe(AudioRouter* router) {
    m_router = router;

    m_in_gain = 1; // 0dB gain by default
    m_in_trim = 1;
    m_48v_state = false;

    construct_hw_packet(0);
}

void AudioInPipe::process_samples(std::span<float>& audio_data) {
    for (auto& s : audio_data) {
        s = (s * m_in_trim);
    }
}

void AudioInPipe::send_ctrl_packet_to_preamp() {
    PreampControl pre{m_in_gain, m_48v_state};

    m_hw_control.packet_data.channel = get_channel();
    memcpy(&m_hw_control.packet_data.data, &pre, sizeof(PreampControl));
    m_router->send_control_packet(m_hw_control, 10);
}

void AudioInPipe::set_gain_lin(float gain) {
    m_in_gain = gain;
    send_ctrl_packet_to_preamp();
}

void AudioInPipe::set_trim_lin(float trim) {
    m_in_trim = trim;
}

void AudioInPipe::set_48v_en(bool state) {
    m_48v_state = state;
    send_ctrl_packet_to_preamp();
}

void AudioInPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        GainTrim gt{};
        memcpy(&gt, &pck.packet_data.data, sizeof(GainTrim));

        float new_trim = gt.trim;
        float new_gain = gt.gain;

        set_gain_lin(new_gain);
        set_trim_lin(new_trim);
        set_48v_en(gt.en_48v);
    }
}

void AudioInPipe::construct_hw_packet(uint8_t channel) {
    m_hw_control.header.type = PacketType::CONTROL;
    m_hw_control.packet_data.channel = channel;
    m_hw_control.packet_data.control_id = 0;
    m_hw_control.packet_data.control_type = DataTypes::CUSTOM;
    m_hw_control.packet_data.elem_index = 0; // Unused for hardware control
}
