// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
        if (m_delay_counter > 0) {
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
    int delta_delay = nsample - m_current_delay;
    m_current_delay = nsample;


    if (delta_delay > 0) {
        // Zero padding
        m_delay_counter = nsample;
    } else {
        // Sample removal
        for (int i = 0; i < std::abs(delta_delay); i++) {
            if (!m_sample_buffer.empty()) {
                m_sample_buffer.pop();
            } else {
                break;
            }
        }
    }
}
