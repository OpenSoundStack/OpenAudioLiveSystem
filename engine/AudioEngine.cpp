#include "AudioEngine.h"

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

void AudioEngine::feed_pipe(AudioPacket &packet) {
    for (auto& pipe : m_pipes) {
        if (pipe->is_pipe_enabled() && pipe->get_channel() == packet.packet_data.channel) {
            pipe->feed_packet(packet);
        }
    }
}

std::optional<uint8_t> AudioEngine::install_pipe(std::shared_ptr<AudioPipe> audio_pipe) {
    // Find next unused pipe
    uint8_t channel_id = 0;
    for (auto& pipe : m_pipes) {
        if (!pipe->is_pipe_enabled()) {
            // Replace old pipe with the new one
            pipe = audio_pipe;
            pipe->set_channel(channel_id);
            pipe->set_pipe_enabled(true);

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

