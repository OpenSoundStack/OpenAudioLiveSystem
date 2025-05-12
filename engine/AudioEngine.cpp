#include "AudioEngine.h"

AudioEngine::AudioEngine() {

}

InitStatus AudioEngine::init_engine() {
    // Init empty disabled pipes
    for (auto& pipe : m_pipes) {
        pipe = std::make_unique<PipeWrapper>(std::optional<std::unique_ptr<AudioPipe>>{});
    }

    return InitStatus::INIT_OK;
}


void AudioEngine::update_pipes() {
    // Top pipe is signal source. Must send value zero. Sample is acquired over network by the first piping stage
    for (auto& pipe : m_pipes) {
        pipe->passthrough_sample(0.0f);
    }
}

