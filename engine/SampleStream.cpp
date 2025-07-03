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

#include "SampleStream.h"

SampleStream::SampleStream() {
    m_current_delay = 0;
    m_delay_counter = 0;
}

void SampleStream::insert_packet(AudioPacket &pck) {
    for (auto& e : pck.packet_data.samples) {
        m_sample_buffer.emplace(e);
    }
}

float SampleStream::pull_sample() {
    if (m_sample_buffer.empty() || (m_delay_counter != 0)) {
        // Delay management
        if (m_delay_counter != 0) {
            m_delay_counter--;
        }

        return 0.0f;
    }

    float oldest_sample = m_sample_buffer.front();
    m_sample_buffer.pop();

    return oldest_sample;
}

bool SampleStream::can_pull() {
    return !m_sample_buffer.empty();
}

size_t SampleStream::queue_size() {
    return m_sample_buffer.size();
}

void SampleStream::time_align(int nsample) {
    // We use the delta between old delay and new delay to know how much
    // we have to increase delay (by inserting zeros) or deleting samples in stream
    m_current_delay = nsample - m_current_delay;

    if (m_current_delay > 0) {
        // Zero padding
        m_delay_counter = nsample;
    } else {
        // Sample removal
        for (int i = 0; i < std::abs(m_current_delay); i++) {
            m_sample_buffer.pop();
        }
    }
}
