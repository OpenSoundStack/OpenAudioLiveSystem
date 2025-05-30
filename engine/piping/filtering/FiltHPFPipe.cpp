#include "FiltHPFPipe.h"

FiltHPFPipe::FiltHPFPipe() : m_filter(100.0f, 96000.0f) {

}

float FiltHPFPipe::process_sample(float sample) {
    return m_filter.push_sample(sample);
}

void FiltHPFPipe::set_filter_cutoff(float cutoff) {
    m_filter.set_cutoff(cutoff);
}

void FiltHPFPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        float cutoff = 0.0f;
        memcpy(&cutoff, pck.packet_data.data, sizeof(float));

        set_filter_cutoff(cutoff);
    }
}
