#include "AudioEngine.h"

AudioEngine::AudioEngine() {

}

InitStatus AudioEngine::init_engine() {
    // Init empty disabled pipes
    int channel = 1;
    for (auto& pipe : m_pipes) {
        pipe = std::make_unique<PipeWrapper>(std::optional<std::unique_ptr<AudioPipe>>{}, channel);
        channel++;
    }

    return InitStatus::INIT_OK;
}


void AudioEngine::update_pipes() {
    // Top pipe is signal source. Must send value zero. Sample is acquired over network by the first piping stage
    for (auto& pipe : m_pipes) {
        pipe->passthrough_sample(0.0);
    }
}

void AudioEngine::feed_pipe(const AudioPacket &packet) {
    for (auto& pipe : m_pipes) {

        if (pipe->pipe_enabled() && pipe->get_channel() == packet.packet_data.channel) {

            for (auto s : packet.packet_data.samples) {
                pipe->passthrough_sample(s);
            }
        }
    }
}

void AudioEngine::install_pipe(uint8_t channel, std::unique_ptr<AudioPipe> audio_pipe) {
    for (auto& pipe : m_pipes) {
        if (!pipe->pipe_enabled()) {
            pipe->set_pipe_enabled(true);
            pipe->install_pipe(std::move(audio_pipe));
            pipe->set_channel(channel);

            break;
        }
    }
}

