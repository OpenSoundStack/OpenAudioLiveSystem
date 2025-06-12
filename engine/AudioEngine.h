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

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#define AUDIO_ENGINE_MAX_PIPES 64

#include <array>
#include <cmath>
#include <memory>
#include <iostream>

#include "engine/piping/io/AudioInPipe.h"
#include "engine/piping/AudioPipe.h"
#include "OpenAudioNetwork/common/packet_structs.h"

#include "log.h"

enum InitStatus {
    INIT_OK = 0,
    INIT_PIPE_INIT_FAIL = 1
};

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine() = default;

    InitStatus init_engine();

    void feed_pipe(AudioPacket& packet);
    void propagate_control(ControlPacket& pck);
    void update_processes();
    std::optional<uint8_t> install_pipe(std::shared_ptr<AudioPipe> audio_pipe);

    bool reset_pipes();

    uint64_t get_channel_usage_map();
private:
    std::array<std::shared_ptr<AudioPipe>, AUDIO_ENGINE_MAX_PIPES> m_pipes;
};

#endif //AUDIOENGINE_H
