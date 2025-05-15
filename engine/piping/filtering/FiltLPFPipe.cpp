#include "FiltLPFPipe.h"

FiltLPFPipe::FiltLPFPipe() : m_filter(1000.0f, 96000.0f) {

}

float FiltLPFPipe::process_sample(float sample) {
    return m_filter.push_sample(sample);
}

void FiltLPFPipe::set_filter_cutoff(float cutoff) {
    m_filter.set_cutoff(cutoff);
}

