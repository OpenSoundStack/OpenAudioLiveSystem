#include "AudioInPipe.h"

AudioInPipe::AudioInPipe() {
    m_in_gain = 1; // 0dB gain by default
    m_in_trim = 1;
}

float AudioInPipe::process_sample(float sample) {
    return (sample * m_in_gain * m_in_trim);
}

void AudioInPipe::set_gain_lin(float gain) {
    m_in_gain = gain;
}

void AudioInPipe::set_trim_lin(float trim) {
    m_in_trim = trim;
}

void AudioInPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        GainTrim gt{};
        memcpy(&gt, &pck.packet_data.data, sizeof(GainTrim));

        float new_trim = gt.trim;
        float new_gain = gt.gain;

        set_gain_lin(new_gain);
        set_trim_lin(new_trim);
    }
}
