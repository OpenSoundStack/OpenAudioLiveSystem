// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#include "CoreEqControlUI.h"

CoreEqControlUI::CoreEqControlUI() : m_filter(1000.0f, 2.0f, 0.0f, 96000.0f) {
    add_handle(100, 0);
    add_handle(1000, 0);

    calc_filter_mag();
}

void CoreEqControlUI::set_cutoff(float fc, int handle_idx) {
    m_handles[handle_idx].fc = fc;
    m_filter.set_cutoff(fc);
    calc_filter_mag();
}

void CoreEqControlUI::set_gain(float gain, int handle_idx) {
    m_handles[handle_idx].gain = gain;
    m_filter.set_gain(gain);
    calc_filter_mag();
}

void CoreEqControlUI::calc_filter_mag() {
    m_filter_mag.clear();

    float Q = 2.0f;
    int npoints = 500;
    for (int i = 0; i < npoints; i++) {
        float f = log_scale_to_freq(i * 4.0f / npoints);

        // GPT-generated
        float A = powf(10.0f, m_handles[0].gain / 20.0f);
        float ratio = f / m_handles[0].fc;
        float term = ratio - 1.0f / ratio;

        // Facteur de correction de largeur
        float p = m_handles[0].gain > 0.0f ? 40.0f : 16.0f;
        float k = 4.0f * powf(10.0f, -fabsf(m_handles[0].gain) / p);

        float H = 1.0f + (A - 1.0f) / (1.0f + k * Q * Q * term * term);

        m_filter_mag.emplace_back(f, H);
    }

}

void CoreEqControlUI::draw_approx_filter(QPainter *painter, QRect zone) {

}
