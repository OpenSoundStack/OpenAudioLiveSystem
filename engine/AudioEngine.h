#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#define AUDIO_ENGINE_MAX_PIPES 64

#include <array>
#include <cmath>

#include "piping/PipeWrapper.h"

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

    void update_pipes();
    void feed_pipe(const AudioPacket& packet);

    void install_pipe(uint8_t channel, std::unique_ptr<AudioPipe> audio_pipe);
private:
    std::array<std::unique_ptr<PipeWrapper>, AUDIO_ENGINE_MAX_PIPES> m_pipes;
};

#endif //AUDIOENGINE_H
