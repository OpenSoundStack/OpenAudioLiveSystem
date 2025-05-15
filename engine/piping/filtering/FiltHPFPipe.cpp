#include "FiltHPFPipe.h"

FiltHPFPipe::FiltHPFPipe() : m_filter(100.0f, 96000.0f) {

}

float FiltHPFPipe::process_sample(float sample) {
    return m_filter.push_sample(sample);
}

void FiltHPFPipe::set_filter_cutoff(float cutoff) {
    m_filter.set_cutoff(cutoff);
}

