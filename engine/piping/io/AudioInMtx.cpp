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

#include "AudioInMtx.h"

AudioInMtx::AudioInMtx() {
    m_pending_packet = AudioPacket{};
    m_pending_packet.packet_data.source_channel = 0;
    m_pending_packet.packet_data.channel = get_channel();

    m_last_sample_idx = 0;
}

void AudioInMtx::feed_packet(AudioPacket &pck) {
    if (!m_streams.contains(pck.packet_data.source_channel)) {
        std::cout << "New stream incoming from channel " << (int)pck.packet_data.source_channel << std::endl;

        SampleStream new_stream{};
        new_stream.insert_packet(pck);
        m_streams[pck.packet_data.source_channel] = std::move(new_stream);
    }

    m_streams[pck.packet_data.source_channel].insert_packet(pck);
}

void AudioInMtx::continuous_process() {
    if (m_streams.empty()) {
        return;
    }

    bool must_break = false;

    while (!must_break) {
        float summed_sample = 0.0f;
        for (auto& s : m_streams) {
            if (s.second.can_pull()) {
                summed_sample += s.second.pull_sample();
            } else {
                must_break = true;
            }
        }

        m_pending_packet.packet_data.samples[m_last_sample_idx] = summed_sample;
        m_last_sample_idx++;

        if (m_last_sample_idx == 64) {
            m_last_sample_idx = 0;
            forward_sample(m_pending_packet);
        }
    }
}


