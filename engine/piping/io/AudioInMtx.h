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

class AudioInMtx : public AudioPipe {
public:
    AudioInMtx();
    ~AudioInMtx() override = default;

    void push_packet(AudioPacket &pck) override;
    void continuous_process() override;
private:
    std::unordered_map<uint32_t, SampleStream> m_streams;

    AudioPacket m_pending_packet;
    int m_last_sample_idx;
};



#endif //AUDIOINMTX_H
