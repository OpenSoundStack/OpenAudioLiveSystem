// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#define AUDIO_ENGINE_MAX_PIPES 64

#include <array>
#include <atomic>
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

    void feed_pipe(AudioPacket& packet, uint16_t src_uid);
    void propagate_control(ControlPacket& pck);
    void update_processes();
    std::optional<uint8_t> install_pipe(std::shared_ptr<AudioPipe> audio_pipe);

    bool reset_pipes();

    uint64_t get_channel_usage_map();

    // Input routing table. For each local pipe, optionally accept audio
    // from a specific (src_uid, src_ch) tuple. feed_pipe() consults this
    // on every incoming AudioPacket. Pipes with no entry receive nothing
    // (so by default a freshly-started engine is deaf — the UI's Routing
    // page is what writes entries here).
    //
    // Encoding: 0 = no route. Otherwise bits 0-15 = src_uid, bits 16-23
    // = src_ch, bits 24-31 unused. (src_uid=0 isn't a valid OAN peer, so
    // 0-means-empty is unambiguous.)
    void set_input_route(uint8_t dest_pipe, uint16_t src_uid, uint8_t src_ch);
    void clear_input_route_by_pipe(uint8_t dest_pipe);
    void clear_input_route_by_source(uint16_t src_uid, uint8_t src_ch);
    void clear_all_input_routes();

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

    // 0 = no route. Otherwise (src_ch << 16) | src_uid. Atomic for
    // lock-free reads on the audio hot path; writes happen from the
    // control-query callback thread on UI clicks.
    std::array<std::atomic<uint32_t>, AUDIO_ENGINE_MAX_PIPES> m_input_routes{};

#ifdef OAN_HOST_BACKENDS
    std::mutex              m_wakeup_mtx;
    std::condition_variable m_wakeup_cv;
    bool                    m_block_ready{false};
#endif
};

#endif //AUDIOENGINE_H
