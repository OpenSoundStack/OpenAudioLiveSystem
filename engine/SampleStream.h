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

#include <array>

#include "OpenAudioNetwork/common/packet_structs.h"

class SampleStream {
public:
    SampleStream();
    ~SampleStream() = default;

    void insert_packet(AudioPacket& pck);
    float pull_sample();

    bool can_pull();
private:
    std::array<float, 1024> m_sample_buffer;

    int m_read_cursor;
    int m_write_cursor;
};



#endif //SAMPLESTREAM_H
