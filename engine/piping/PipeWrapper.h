#ifndef PIPEWRAPPER_H
#define PIPEWRAPPER_H

#include "OpenAudioNetwork/common/AudioPipe.h"

#include <memory>

class PipeWrapper {
public:
    PipeWrapper(std::optional<std::unique_ptr<AudioPipe>> base_pipe, uint8_t channel_no);
    ~PipeWrapper() = default;

    void install_pipe(std::unique_ptr<AudioPipe> pipe);

    void passthrough_sample(float sample);

    void set_pipe_enabled(bool en);
    bool pipe_enabled() const;

    void set_channel(uint8_t channel);
    uint8_t get_channel();
private:
    std::optional<std::unique_ptr<AudioPipe>> m_base_pipe;
    bool m_pipe_enabled;

    uint8_t m_channel_no;
};



#endif //PIPEWRAPPER_H
