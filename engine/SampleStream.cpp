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

}

void SampleStream::insert_packet(AudioPacket &pck) {
    for (auto& e : pck.packet_data.samples) {
        m_sample_buffer.emplace(e);
    }
}

float SampleStream::pull_sample() {
    if (m_sample_buffer.empty()) {
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
