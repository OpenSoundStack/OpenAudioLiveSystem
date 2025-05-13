#include "AudioPlumber.h"

AudioPlumber::AudioPlumber() {

}

bool AudioPlumber::do_elem_exists(const std::string &elem) {
    return m_elem_map.find(elem) != m_elem_map.end();
}


void AudioPlumber::register_pipe_element(const std::string& elem_name, const std::function<std::unique_ptr<AudioPipe>()>& factory) {
    m_elem_map[elem_name] = factory;
}


std::optional<std::unique_ptr<AudioPipe>> AudioPlumber::construct_pipe(const std::vector<std::string>& pipeline) {
    if (pipeline.empty()) {
        return {};
    }

    if (!do_elem_exists(pipeline[0])) {
        return {};
    }

    std::unique_ptr<AudioPipe> root_pipe = m_elem_map[pipeline[0]]();

    for (int i = 1; i < pipeline.size(); i++) {
        if (!do_elem_exists(pipeline[i])) {
            return {};
        }

        auto pipe_elem = m_elem_map[pipeline[i]]();
        root_pipe->set_next_pipe(std::move(pipe_elem));
    }

    return root_pipe;
}
