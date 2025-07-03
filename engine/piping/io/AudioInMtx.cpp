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

void AudioInMtx::push_packet(AudioPacket &pck) {
    if (!m_streams.contains(pck.packet_data.source_channel)) {
        std::cout << "New stream incoming from channel " << (int)pck.packet_data.source_channel << std::endl;

        std::unique_lock<std::mutex> __lock{m_lock};
        SampleStream new_stream{};
        new_stream.insert_packet(pck);
        m_streams[pck.packet_data.source_channel] = new_stream;
        m_lat_data[pck.packet_data.source_channel] = {0, 0.0f};

        return;
    }

    std::unique_lock<std::mutex> __lock{m_lock};
    m_streams[pck.packet_data.source_channel].insert_packet(pck);

    time_align_routine(pck);
}

void AudioInMtx::time_align_routine(AudioPacket& pck) {
    static auto last_lat_meas = NetworkMapper::local_now_us();

    auto now = NetworkMapper::local_now_us();
    int64_t delta = now - pck.header.timestamp;

    auto& lat_data = m_lat_data[pck.packet_data.source_channel];
    lat_data.sample_count++;
    lat_data.sample_sum += (float)delta;

    if (now - last_lat_meas > 5000000) {
        float min_lat = 0.0f;

        int i = 0;
        for (auto& c : m_lat_data) {
            c.second.lat_mean_us = c.second.sample_sum / c.second.sample_count;

            if (i == 0) {
                min_lat = c.second.lat_mean_us;
            } else if (c.second.lat_mean_us < min_lat) {
                min_lat = c.second.lat_mean_us;
            }

            i++;
        }

#ifdef DEBUG_LOG_LATENCY
        std::cout << "--- LATENCY REPORT ---" << std::endl;
#endif

        for (auto& c : m_lat_data) {
            constexpr int sample_period_us = 1000000 / 96000;

            int req_delay = ((int)(c.second.lat_mean_us - min_lat) / sample_period_us);
            m_streams[c.first].time_align(req_delay);

#ifdef DEBUG_LOG_LATENCY
            std::cout << "Channel " << (int)c.first << ", Meas latency : " << c.second.lat_mean_us << " us";
            std::cout << " delta is " << req_delay << " samples" << std::endl;
#endif
            c.second = {0, 0, 0};
        }

        last_lat_meas = now;
    }
}

void AudioInMtx::continuous_process() {
    if (m_streams.empty()) {
        return;
    }

    // Check if the stream is pull-able with some buffering
    // At least 2 packets in buffer to allow to pull
    // If not enough data is present, immediately return
    // There is a downside though, it increases the latency by (n - 1) packets
    // where n is the number of packets buffered per streams
    constexpr size_t buffer_threshold = 2 * AUDIO_DATA_SAMPLES_PER_PACKETS;
    for (auto& s : m_streams) {
        if (s.second.queue_size() < buffer_threshold) {
            return;
        }
    }

    // Pull at most one packet (because one pipe only send one packet at a time)
    for (int i = 0; i < AUDIO_DATA_SAMPLES_PER_PACKETS; i++) {
        float summed_sample = 0.0f;

        {
            std::unique_lock<std::mutex> __lock{m_lock};
            for (auto& s : m_streams) {
                if (s.second.can_pull()) {
                    summed_sample += s.second.pull_sample();
                }
            }
        }

        m_pending_packet.packet_data.samples[m_last_sample_idx] = summed_sample;
        m_last_sample_idx++;

        if (m_last_sample_idx == AUDIO_DATA_SAMPLES_PER_PACKETS) {
            m_last_sample_idx = 0;
            forward_sample(m_pending_packet);
        }
    }
}


