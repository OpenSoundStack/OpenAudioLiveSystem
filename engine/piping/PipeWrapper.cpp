#include "PipeWrapper.h"

PipeWrapper::PipeWrapper(std::optional<std::unique_ptr<AudioPipe>> base_pipe) {
    m_base_pipe = std::move(base_pipe);
    m_pipe_enabled = false;
}

void PipeWrapper::passthrough_sample(float sample) {
    if (m_pipe_enabled && m_base_pipe.has_value()) {
        m_base_pipe.value()->acquire_sample(sample);
    } else {
        return;
    }
}

void PipeWrapper::set_pipe_enabled(bool en) {
    m_pipe_enabled = en;
}

bool PipeWrapper::pipe_enabled() const {
    return m_pipe_enabled;
}

