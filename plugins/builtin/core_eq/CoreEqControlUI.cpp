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

CoreEqControlUI::CoreEqControlUI() {
    init_filters();
}

void CoreEqControlUI::set_cutoff(float fc, int handle_idx) {
    m_handles[handle_idx].fc = fc;
    m_filters[handle_idx].fc = fc;

    calc_filter_mag();
}

void CoreEqControlUI::set_gain(float gain, int handle_idx) {
    m_handles[handle_idx].gain = gain;
    m_filters[handle_idx].gain = gain;

    calc_filter_mag();
}

void CoreEqControlUI::calc_filter_mag() {
    constexpr int npoints = 500;

    m_filter_mag.clear();

    for (int i = 0; i < npoints; i++) {
        float current_mag = 1.0f;
        float f = log_scale_to_freq(i * 4.0f / npoints);

        for (auto& filter : m_filters) {
            // GPT-generated
            float A = powf(10.0f, filter.gain / 20.0f);
            float ratio = f / filter.fc;
            float term = ratio - 1.0f / ratio;

            // Facteur de correction de largeur
            float p = filter.gain > 0.0f ? 40.0f : 16.0f;
            float k = 4.0f * powf(10.0f, -fabsf(filter.gain) / p);

            float H = 1.0f + (A - 1.0f) / (1.0f + k * filter.Q * filter.Q * term * term);

            current_mag *= H;
        }

        m_filter_mag.emplace_back(f, current_mag);
    }

}

void CoreEqControlUI::draw_approx_filter(QPainter *painter, QRect zone) {

}

void CoreEqControlUI::init_filters() {
    for (int i = 0; i < 6; i++) {
        m_filters[i] = FilterParams{
            .fc = default_frequencies[i],
            .gain = 0.0f,
            .Q = default_Q
        };

        add_handle(default_frequencies[i], 0.0f, handle_colors[i]);
    }

    calc_filter_mag();
}

std::vector<QPointF> CoreEqControlUI::get_eq_curve() {
    return calc_curve(m_filter_mag);
}
