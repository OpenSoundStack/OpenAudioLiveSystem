#include "AudioEngine.h"

AudioEngine::AudioEngine() {

}

InitStatus AudioEngine::init_engine() {
    // Init empty disabled pipes
    int channel = 1;
    for (auto& pipe : m_pipes) {
        pipe = std::make_unique<AudioInPipe>();
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

void AudioEngine::install_pipe(uint8_t channel, std::unique_ptr<AudioPipe> audio_pipe) {
    // Find next unused pipe

    for (auto& pipe : m_pipes) {
        if (!pipe->is_pipe_enabled()) {
            pipe->set_channel(channel);
            pipe->set_next_pipe(std::move(audio_pipe));
            pipe->set_pipe_enabled(true);

            break;
        }
    }
}

