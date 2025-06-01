#ifndef AUDIOPLUMBER_H
#define AUDIOPLUMBER_H

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>

#include "engine/piping/AudioPipe.h"

class AudioPlumber {
public:
    AudioPlumber();
    ~AudioPlumber() = default;

    void register_pipe_element(const std::string& elem_name, const std::function<std::shared_ptr<AudioPipe>()>& factory);
    std::optional<std::shared_ptr<AudioPipe>> construct_pipe(const std::vector<std::string>& pipeline);

    int pending_elem_count() const;

    void set_pending_client(uint16_t client);
    uint16_t get_pending_client() const;

    void add_elem_to_pending_pipe(const std::string& elem, int position);
    std::optional<std::shared_ptr<AudioPipe>> construct_pending_pipe();
    void reset_pending_pipe();
private:
    bool do_elem_exists(const std::string& elem);

    std::unordered_map<std::string, std::function<std::shared_ptr<AudioPipe>()>> m_elem_map;

    std::vector<std::pair<std::string, int>> m_pending_pipe;
    uint8_t m_pending_client;
};



#endif //AUDIOPLUMBER_H
