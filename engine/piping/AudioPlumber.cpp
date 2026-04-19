// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "AudioPlumber.h"

AudioPlumber::AudioPlumber() {

}

bool AudioPlumber::do_elem_exists(const std::string &elem) {
    return m_elem_map.find(elem) != m_elem_map.end();
}


void AudioPlumber::register_pipe_element(const std::string& elem_name, const std::function<std::shared_ptr<AudioPipe>()>& factory) {
    m_elem_map[elem_name] = factory;
}


std::optional<std::shared_ptr<AudioPipe>> AudioPlumber::construct_pipe(const std::vector<std::string>& pipeline) {
    if (pipeline.empty()) {
        return {};
    }

    if (!do_elem_exists(pipeline[0])) {
        return {};
    }

    std::shared_ptr<AudioPipe> root_pipe = m_elem_map[pipeline[0]]();

    std::shared_ptr<AudioPipe> last_elem = root_pipe;
    for (int i = 1; i < pipeline.size(); i++) {
        if (!do_elem_exists(pipeline[i])) {
            return {};
        }

        auto pipe_elem = m_elem_map[pipeline[i]]();
        last_elem->set_next_pipe(pipe_elem);

        last_elem = pipe_elem;
    }

    root_pipe->propagate_index(0);

    return root_pipe;
}

void AudioPlumber::add_elem_to_pending_pipe(const std::string& elem, int position, uint16_t pid, int pipe_size, uint8_t client) {
    auto mapid = construct_id_client_pair(pid, client);

    m_pending_pipe[mapid].pipe_elems.emplace_back(elem, position);
    m_pending_pipe[mapid].pipe_size = pipe_size;
    m_pending_pipe[mapid].pending_client = client;
}

int AudioPlumber::pending_elem_count(uint16_t pid, uint8_t client_id) {
    uint32_t mapid = construct_id_client_pair(pid, client_id);
    return m_pending_pipe[mapid].pipe_elems.size();
}

std::optional<std::shared_ptr<AudioPipe> > AudioPlumber::construct_pending_pipe(uint16_t pid, uint8_t client_id) {
    uint32_t mapid = construct_id_client_pair(pid, client_id);
    auto& ppipe = m_pending_pipe[mapid].pipe_elems;

    // Sort the pipe so the pipe elements are in the desired order
    std::sort(
        ppipe.begin(), ppipe.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second < b.second;
    });

    std::vector<std::string> pline;
    for (auto& elem : ppipe) {
        pline.push_back(elem.first);
    }

    auto pipeline = construct_pipe(pline);
    reset_pending_pipe(pid, client_id);

    return pipeline;
}

void AudioPlumber::reset_pending_pipe(uint16_t pid, uint8_t client_id) {
    uint32_t mapid = construct_id_client_pair(pid, client_id);
    m_pending_pipe.erase(mapid);
}

uint16_t AudioPlumber::get_pending_client(uint16_t pid, uint8_t client_id) {
    uint32_t mapid = construct_id_client_pair(pid, client_id);
    return m_pending_pipe[pid].pending_client;
}

uint32_t AudioPlumber::construct_id_client_pair(uint16_t pid, uint8_t client_uid) {
    return static_cast<uint32_t>(client_uid << 16) | static_cast<uint32_t>(pid);
}
