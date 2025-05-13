#ifndef AUDIOPLUMBER_H
#define AUDIOPLUMBER_H

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>

#include "OpenAudioNetwork/common/AudioPipe.h"

class AudioPlumber {
public:
    AudioPlumber();
    ~AudioPlumber() = default;

    void register_pipe_element(const std::string& elem_name, const std::function<std::unique_ptr<AudioPipe>()>& factory);
    std::optional<std::unique_ptr<AudioPipe>> construct_pipe(const std::vector<std::string>& pipeline);

private:
    bool do_elem_exists(const std::string& elem);

    std::unordered_map<std::string, std::function<std::unique_ptr<AudioPipe>()>> m_elem_map;
};



#endif //AUDIOPLUMBER_H
