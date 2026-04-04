// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
