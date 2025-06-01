#include "AudioPipe.h"

AudioPipe::AudioPipe() {
    m_next_pipe = {};
    m_pipe_enabled = false;
}

void AudioPipe::feed_packet(AudioPacket &pck) {
    if (!m_pipe_enabled) {
        return;
    }

    // Process all sample for the current pipe and
    // then forward to the next.
    // Routing audio packet responsability is lead to the pipe
    // elements.

    for (auto& s : pck.packet_data.samples) {
        s = process_sample(s);
    }

    forward_sample(pck);
}


float AudioPipe::process_sample(float sample) {
    return sample;
}

void AudioPipe::forward_sample(AudioPacket& pck) {
    if (m_next_pipe.has_value()) {
        m_next_pipe.value()->feed_packet(pck);
    }
}

void AudioPipe::set_next_pipe(const std::shared_ptr<AudioPipe>& pipe) {
    m_next_pipe = pipe;
}

bool AudioPipe::is_pipe_enabled() const {
    return m_pipe_enabled;
}

void AudioPipe::set_pipe_enabled(bool en) {
    m_pipe_enabled = en;

    // Propagate to next pipe elem in chain
    if (m_next_pipe.has_value()) {
        m_next_pipe.value()->set_pipe_enabled(en);
    }
}

void AudioPipe::set_channel(uint8_t channel) {
    m_channel_no = channel;

    // Propagate to next pipe elem in chain
    if (m_next_pipe.has_value()) {
        m_next_pipe.value()->set_channel(channel);
    }
}

uint8_t AudioPipe::get_channel() {
    return m_channel_no;
}

std::optional<std::shared_ptr<AudioPipe> > AudioPipe::next_pipe() {
    return m_next_pipe;
}

void AudioPipe::apply_control(ControlPacket &pck) {
    // To be overriden...
}
