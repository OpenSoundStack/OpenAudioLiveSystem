#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <array>

#include "OpenAudioNetwork/common/AudioPipe.h"

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

private:
    std::array<AudioPipe, 64> m_pipes;
};

#endif //AUDIOENGINE_H
