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

#include "AudioSendMtx.h"

AudioSendMtx::AudioSendMtx(AudioRouter *router) : AudioPipe() {
    m_router = router;
}

void AudioSendMtx::feed_packet(AudioPacket &pck) {
    for (auto& fader : m_fader_map) {
        // No need to
        if (fader.second.level == 0.0f) {
            continue;
        }

        // Re-route channel
        AudioPacket pck_cpy = pck;
        pck_cpy.packet_data.source_channel = get_channel();
        pck_cpy.packet_data.channel = fader.second.channel;

        process_packet(pck_cpy, fader.first);
        m_router->send_audio_packet(pck_cpy, fader.second.host);
    }
}

float AudioSendMtx::process_sample(float sample) {
    return sample;
}

void AudioSendMtx::apply_control(ControlPacket &pck) {
    FaderControlFrame fader_frame{};
    memcpy(&fader_frame, pck.packet_data.data, sizeof(FaderControlFrame));

    m_fader_map[fader_frame.channel] = fader_frame;
}

void AudioSendMtx::process_packet(AudioPacket &pck, uint8_t fader_channel) {
    float multiply_value = m_fader_map[fader_channel].level;

    for (auto& sample : pck.packet_data.samples) {
        sample *= multiply_value;
    }
}

