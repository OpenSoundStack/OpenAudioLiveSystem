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

#ifdef OAN_HOST_BACKENDS
#include <condition_variable>
#include <mutex>
#endif

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

#ifdef OAN_HOST_BACKENDS
    // Producer side: called from the audio recv callback after feed_pipe.
    // Wakes the pipe_updater thread so it can drain the block instead of
    // busy-ticking. No-op on Linux — Linux RT thread layout is unchanged.
    void notify_block_ready();

    // Consumer side: pipe_updater blocks here until a block arrives or
    // the timeout elapses. Timeout acts as a heartbeat so time-driven
    // work in continuous_process() (release envelopes, etc.) still runs
    // when the wire is idle. Returns true if woken by notify, false on
    // timeout. timeout_us = 0 means no wait.
    bool wait_for_block(int timeout_us);
#endif

private:
    std::array<std::shared_ptr<AudioPipe>, AUDIO_ENGINE_MAX_PIPES> m_pipes;

#ifdef OAN_HOST_BACKENDS
    std::mutex              m_wakeup_mtx;
    std::condition_variable m_wakeup_cv;
    bool                    m_block_ready{false};
#endif
};

#endif //AUDIOENGINE_H
