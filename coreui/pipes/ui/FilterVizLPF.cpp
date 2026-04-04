// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

//
// Created by mathis on 03/07/25.
//

#include "FilterVizLPF.h"

FilterVizLPF::FilterVizLPF() {
    add_handle(1000, 0);
}

void FilterVizLPF::set_cutoff(float fc, int handle_idx) {
    m_handles[handle_idx].fc = fc;
    update();
}

void FilterVizLPF::draw_approx_filter(QPainter *painter, QRect zone) {
    float freq_x_pos = freq_to_log_scale(m_handles[0].fc) * zone.width();

    // Computing characteristic points for the slope to be coherent
    float freq_10k_x_pos = freq_to_log_scale(10000.0f) * zone.width();
    float freq_1k_x_pos = freq_to_log_scale(1000.0f) * zone.width();
    float decade_distance = freq_10k_x_pos - freq_1k_x_pos;

    float freq2_x_pos = freq_x_pos + decade_distance;

    float stopband_level = -36; // -36 dB
    stopband_level = (stopband_level + 18.0f) / 36.0f;
    float stopband_y = zone.height() - stopband_level * zone.height();

    QPainterPath path{};
    path.moveTo(QPoint{(int)freq2_x_pos, (int)stopband_y});
    path.lineTo(QPoint{(int)freq_x_pos, zone.height() / 2});
    path.lineTo(QPoint{0, zone.height() / 2});

    painter->drawPath(path);
}

