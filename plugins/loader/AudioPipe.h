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

#ifndef AUDIOPIPE_H
#define AUDIOPIPE_H

#include <memory>
#include <optional>
#include <queue>
#include <mutex>
#include <mutex>

#include "../../OpenAudioNetwork/common/packet_structs.h"

/**
 * @class AudioPipe
 * @brief Represents a list of pipe elements. Each pipe element does one processing. The whole pipeline forms the processing chain.
 */
class AudioPipe {
public:
    AudioPipe();
    virtual ~AudioPipe() = default;

    /**
     * Direct feed the pipe, bypasses queue
     * @param pck An audio packet
     */
    virtual void feed_packet(AudioPacket& pck);

    /**
     *  Pushes a packet in queue
     */
    virtual void push_packet(AudioPacket& pck);

    /**
     * Process oldest packet in queue
     */
    void process_next_packet();

    /**
     * Install nex pipe element in chain
     * @param pipe Next pipe
     */
    void set_next_pipe(const std::shared_ptr<AudioPipe>& pipe);

    /**
     * If it exists, returns the next pipe elem in chain
     * @return
     */
    std::optional<std::shared_ptr<AudioPipe>> next_pipe();

    bool is_pipe_enabled() const;
    void set_pipe_enabled(bool en);

    void set_channel(uint8_t channel);
    uint8_t get_channel();

    /**
     * Apply a control packet to the elem
     * @param pck
     */
    virtual void apply_control(ControlPacket& pck);

    /**
     * Executes at each polling
     */
    virtual void continuous_process();

protected:
    /**
     * Sends processed audio to the next pipe element
     * @param pck Processed audio packet
     */
    void forward_sample(AudioPacket& pck);

    /**
     * Process a unique sample
     * @param sample Audio sample
     * @return The processed sample
     */
    virtual float process_sample(float sample);

    std::mutex m_lock;

private:
    std::optional<std::shared_ptr<AudioPipe>> m_next_pipe;

    std::queue<AudioPacket> m_packet_queue;

    // Pipe meta info
    bool m_pipe_enabled;
    uint8_t m_channel_no;
};



#endif //AUDIOPIPE_H
