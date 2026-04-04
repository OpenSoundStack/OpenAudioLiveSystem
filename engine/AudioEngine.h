// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#define AUDIO_ENGINE_MAX_PIPES 64

#include <array>
#include <cmath>
#include <memory>
#include <iostream>

#include "plugins/loader/AudioPipe.h"
#include "engine/piping/io/AudioInPipe.h"
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
