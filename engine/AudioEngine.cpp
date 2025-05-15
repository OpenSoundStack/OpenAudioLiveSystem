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

void AudioEngine::install_pipe(uint8_t channel, std::shared_ptr<AudioPipe> audio_pipe) {
    // Find next unused pipe

    for (auto& pipe : m_pipes) {
        if (!pipe->is_pipe_enabled()) {
            // Replace old pipe with the new one
            pipe = audio_pipe;
            pipe->set_channel(channel);
            pipe->set_pipe_enabled(true);

            break;
        }
    }
}
