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

#ifndef SAMPLESTREAM_H
#define SAMPLESTREAM_H

#include <queue>
#include <shared_mutex>
#include <mutex>

#include "OpenAudioNetwork/common/packet_structs.h"

class SampleStream {
public:
    SampleStream();
    ~SampleStream() = default;

    void insert_packet(AudioPacket& pck);
    float pull_sample();

    bool can_pull();

    size_t queue_size();
private:
    std::queue<float> m_sample_buffer;
};



#endif //SAMPLESTREAM_H
