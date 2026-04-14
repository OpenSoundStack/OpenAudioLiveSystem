// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef AUDIOPLUMBER_H
#define AUDIOPLUMBER_H

#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>

#include "plugins/loader/AudioPipe.h"

struct PendingPipeData {
    std::vector<std::pair<std::string, int>> pipe_elems;
    int pipe_size;
    uint8_t pending_client;
};

class AudioPlumber {
public:
    AudioPlumber();
    ~AudioPlumber() = default;

    void register_pipe_element(const std::string& elem_name, const std::function<std::shared_ptr<AudioPipe>()>& factory);
    std::optional<std::shared_ptr<AudioPipe>> construct_pipe(const std::vector<std::string>& pipeline);

    int pending_elem_count(uint16_t pid, uint8_t client_id);
    uint16_t get_pending_client(uint16_t pid, uint8_t client_id);

    void add_elem_to_pending_pipe(const std::string& elem, int position, uint16_t pid, int pipe_size, uint8_t client);
    std::optional<std::shared_ptr<AudioPipe>> construct_pending_pipe(uint16_t pid, uint8_t client_id);
    void reset_pending_pipe(uint16_t pid, uint8_t client_id);
private:
    bool do_elem_exists(const std::string& elem);
    static uint32_t construct_id_client_pair(uint16_t pid, uint8_t client_uid);

    std::unordered_map<std::string, std::function<std::shared_ptr<AudioPipe>()>> m_elem_map;

    std::unordered_map<uint32_t, PendingPipeData> m_pending_pipe;
};



#endif //AUDIOPLUMBER_H
