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

    return root_pipe;
}

void AudioPlumber::add_elem_to_pending_pipe(const std::string& elem, int position) {
    m_pending_pipe.emplace_back(elem, position);
}

int AudioPlumber::pending_elem_count() const {
    return m_pending_pipe.size();
}

std::optional<std::shared_ptr<AudioPipe> > AudioPlumber::construct_pending_pipe() {
    // Sort the pipe so the pipe elements are in the desired order
    std::sort(
        m_pending_pipe.begin(), m_pending_pipe.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
        return a.second < b.second;
    });

    std::vector<std::string> pline;
    for (auto& elem : m_pending_pipe) {
        pline.push_back(elem.first);
    }

    auto pipeline = construct_pipe(pline);
    reset_pending_pipe();

    return pipeline;
}

void AudioPlumber::reset_pending_pipe() {
    m_pending_pipe.clear();
}

int AudioPlumber::get_pending_channel() const {
    return m_pending_channel;
}

void AudioPlumber::set_pending_channel(uint8_t channel) {
    m_pending_channel = channel;
}

