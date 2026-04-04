// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef SAMPLESTREAM_H
#define SAMPLESTREAM_H

#include <queue>

#include "OpenAudioNetwork/common/packet_structs.h"

class SampleStream {
public:
    SampleStream();
    ~SampleStream() = default;

    void insert_packet(AudioPacket& pck);
    float pull_sample();

    bool can_pull();

    size_t queue_size();

    void time_align(int nsample);
private:
    std::queue<float> m_sample_buffer;

    int m_delay_counter;
    int m_current_delay;
};



#endif //SAMPLESTREAM_H
