#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#define AUDIO_ENGINE_MAX_PIPES 64

#include <array>
#include <cmath>
#include <memory>

#include "OpenAudioNetwork/common/base_pipes/AudioInPipe.h"
#include "OpenAudioNetwork/common/AudioPipe.h"
#include "OpenAudioNetwork/common/packet_structs.h"

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
    void install_pipe(uint8_t channel, std::shared_ptr<AudioPipe> audio_pipe);
private:
    std::array<std::shared_ptr<AudioPipe>, AUDIO_ENGINE_MAX_PIPES> m_pipes;
};

#endif //AUDIOENGINE_H
