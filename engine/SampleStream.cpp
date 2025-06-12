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
    m_read_cursor = 0;
    m_write_cursor = 0;
}

void SampleStream::insert_packet(AudioPacket &pck) {
    if (m_write_cursor < (1024 - AUDIO_DATA_SAMPLES_PER_PACKETS)) {
        memcpy(m_sample_buffer.data() + m_write_cursor, pck.packet_data.samples, sizeof(float) * AUDIO_DATA_SAMPLES_PER_PACKETS);
        m_write_cursor += AUDIO_DATA_SAMPLES_PER_PACKETS;
    } else {
        int until_end = 1024 - m_write_cursor;
        int remaining = AUDIO_DATA_SAMPLES_PER_PACKETS - until_end;

        memcpy(m_sample_buffer.data() + m_write_cursor, pck.packet_data.samples, sizeof(float) * until_end);
        memcpy(m_sample_buffer.data(), pck.packet_data.samples + until_end, sizeof(float) * remaining);

        m_write_cursor = remaining;
    }
}

float SampleStream::pull_sample() {
    // No data in buffer
    if (m_write_cursor == m_read_cursor) {
        return 0.0f;
    }

    float sample = m_sample_buffer[m_read_cursor];
    m_read_cursor = (m_read_cursor + 1) % 1024;

    return sample;
}

bool SampleStream::can_pull() {
    return m_write_cursor != m_read_cursor;
}

