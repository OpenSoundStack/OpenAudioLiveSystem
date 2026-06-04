// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "AudioEngine.h"

#ifdef OAN_HOST_BACKENDS
#include <chrono>
#endif

AudioEngine::AudioEngine() {

}

InitStatus AudioEngine::init_engine() {
    // Init empty disabled pipes
    int channel = 1;
    for (auto& pipe : m_pipes) {
        pipe = std::make_shared<AudioPipe>();
        channel++;
    }

    return InitStatus::INIT_OK;
}

void AudioEngine::feed_pipe(AudioPacket &packet, uint16_t src_uid) {
    // Build the route key once and walk the input-route table. The key
    // packs the source UID and the source channel field; a pipe is fed
    // iff its entry matches.
    const uint32_t key = (static_cast<uint32_t>(packet.packet_data.channel) << 16)
                        | static_cast<uint32_t>(src_uid);

    for (size_t i = 0; i < m_pipes.size(); ++i) {
        if (m_input_routes[i].load(std::memory_order_relaxed) != key) continue;
        auto& pipe = m_pipes[i];
        if (pipe->is_pipe_enabled()) {
            pipe->push_packet(packet);
        }
    }
#ifdef OAN_HOST_BACKENDS
    notify_block_ready();
#endif
}

void AudioEngine::set_input_route(uint8_t dest_pipe, uint16_t src_uid, uint8_t src_ch) {
    if (dest_pipe >= m_input_routes.size()) return;
    if (src_uid == 0) return; // 0 == "no route" sentinel.
    const uint32_t key = (static_cast<uint32_t>(src_ch) << 16) | static_cast<uint32_t>(src_uid);
    // A pipe can only sink from one source (enforced by the slot being a
    // single value), but a source may legitimately fan out to many
    // pipes (e.g. splitting one mic across several DSP chains), so we
    // do NOT erase other slots that share the same key.
    m_input_routes[dest_pipe].store(key, std::memory_order_relaxed);
}

void AudioEngine::clear_input_route_by_pipe(uint8_t dest_pipe) {
    if (dest_pipe >= m_input_routes.size()) return;
    m_input_routes[dest_pipe].store(0, std::memory_order_relaxed);
}

void AudioEngine::clear_input_route_by_source(uint16_t src_uid, uint8_t src_ch) {
    if (src_uid == 0) return;
    const uint32_t key = (static_cast<uint32_t>(src_ch) << 16) | static_cast<uint32_t>(src_uid);
    for (auto& slot : m_input_routes) {
        if (slot.load(std::memory_order_relaxed) == key) {
            slot.store(0, std::memory_order_relaxed);
        }
    }
}

void AudioEngine::clear_all_input_routes() {
    for (auto& slot : m_input_routes) {
        slot.store(0, std::memory_order_relaxed);
    }
}

std::optional<uint8_t> AudioEngine::install_pipe(std::shared_ptr<AudioPipe> audio_pipe) {
    // Find next unused pipe
    uint8_t channel_id = 0;
    for (auto& pipe : m_pipes) {
        if (!pipe->is_pipe_enabled()) {
            // Replace old pipe with the new one. Wipe any stale route
            // entry the previous occupant left behind so the new pipe
            // doesn't silently inherit incoming audio.
            pipe = audio_pipe;
            pipe->set_channel(channel_id);
            pipe->set_pipe_enabled(true);
            m_input_routes[channel_id].store(0, std::memory_order_relaxed);

            return channel_id;
        }

        channel_id++;
    }

    return {};
}

uint64_t AudioEngine::get_channel_usage_map() {
    uint64_t usage_map = 0;

    for (int i = 0; i < m_pipes.size(); i++) {
        if (!m_pipes[i]->is_pipe_enabled()) {
            usage_map |= ((uint64_t)1 << i);
        }
    }

    return usage_map;
}

bool AudioEngine::reset_pipes() {
    for (auto& elem : m_pipes) {
        // Resetting pipes
        elem = std::make_shared<AudioPipe>();
    }

    return true;
}

void AudioEngine::propagate_control(ControlPacket &pck) {
    if (pck.packet_data.channel >= 64) {
        std::cerr << LOG_PREFIX << "Unknown channel " << (int)pck.packet_data.channel << std::endl;
    }

    auto pipe = m_pipes[pck.packet_data.channel];
    int index = 0;

    while (pck.packet_data.elem_index != index) {
        auto next = pipe->next_pipe();
        if (next.has_value()) {
            pipe = next.value();
        } else {
            std::cerr << LOG_PREFIX << "Failed to find pipe elem at index " << pck.packet_data.elem_index << std::endl;
        }

        index++;
    }

    pipe->apply_control(pck);
}

void AudioEngine::update_processes() {
    for (auto& p : m_pipes) {
        if (p->is_pipe_enabled()) {
            p->process_next_packet();
            p->continuous_process();
        }
    }
}

#ifdef OAN_HOST_BACKENDS
void AudioEngine::notify_block_ready() {
    {
        std::lock_guard<std::mutex> lk{m_wakeup_mtx};
        m_block_ready = true;
    }
    m_wakeup_cv.notify_one();
}

bool AudioEngine::wait_for_block(int timeout_us) {
    std::unique_lock<std::mutex> lk{m_wakeup_mtx};
    bool woken = m_wakeup_cv.wait_for(
        lk, std::chrono::microseconds(timeout_us),
        [this]() { return m_block_ready; });
    m_block_ready = false;
    return woken;
}
#endif

