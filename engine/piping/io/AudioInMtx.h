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

#ifndef AUDIOINMTX_H
#define AUDIOINMTX_H

#include "plugins/loader/AudioPipe.h"
#include "engine/SampleStream.h"

#include <OpenAudioNetwork/common/NetworkMapper.h>

#include <unordered_map>
#include <iostream>

struct LatencyData {
    int sample_count;
    float sample_sum;
    float lat_mean_us;
};

class AudioInMtx : public AudioPipe {
public:
    AudioInMtx();
    ~AudioInMtx() override = default;

    void push_packet(AudioPacket &pck) override;
    void continuous_process() override;
private:
    void time_align_routine(AudioPacket& pck);

    std::unordered_map<uint32_t, SampleStream> m_streams;
    std::unordered_map<uint32_t, LatencyData> m_lat_data;

    AudioPacket m_pending_packet;
    int m_last_sample_idx;

    int m_max_proc_delay_us;
    int64_t m_last_lat_meas;
};



#endif //AUDIOINMTX_H
