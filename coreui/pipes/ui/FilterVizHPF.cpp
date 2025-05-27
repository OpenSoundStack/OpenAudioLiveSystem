#include "FilterVizHPF.h"

FilterVizHPF::FilterVizHPF() : m_filter(100.0f, 0.707f, 96000.0f) {
    FilterVizHPF::set_cutoff(100.0f);
}

void FilterVizHPF::set_cutoff(float fc) {
    m_fc = fc;
    m_filter.set_cutoff(fc);
    calc_filter_mag();
}

void FilterVizHPF::calc_filter_mag() {
    m_filter_mag.clear();

    for (int base = 10; base <= 10000; base *= 10) {
        for (int i = 10; i < 90; i += 1) {
            float freq = (i / 10.0f) * base;
            float rel_freq = freq / 96000.0f;

            float mag = m_filter.get_filter().freq_response_magnitude(rel_freq);

            m_filter_mag.push_back({freq, mag});
        }
    }
}
