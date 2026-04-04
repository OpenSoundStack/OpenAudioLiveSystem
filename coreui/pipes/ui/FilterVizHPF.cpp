// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "FilterVizHPF.h"

FilterVizHPF::FilterVizHPF() {
    add_handle(100.0f, 0);
}

void FilterVizHPF::set_cutoff(float fc, int handle_idx) {
    m_handles[handle_idx].fc = fc;
    update();
}

void FilterVizHPF::draw_approx_filter(QPainter *painter, QRect zone) {
    QPainterPath path{};
    path.moveTo(QPoint{zone.width(), zone.height() / 2});

    float freq_x_pos = freq_to_log_scale(m_handles[0].fc) * zone.width();

    // Computing characteristic points for the slope to be coherent
    float freq_10k_x_pos = freq_to_log_scale(10000.0f) * zone.width();
    float freq_1k_x_pos = freq_to_log_scale(1000.0f) * zone.width();
    float decade_distance = freq_10k_x_pos - freq_1k_x_pos;

    float freq2_x_pos = freq_x_pos - decade_distance;

    float stopband_level = -36; // -36 dB
    stopband_level = (stopband_level + 18.0f) / 36.0f;
    float stopband_y = zone.height() - stopband_level * zone.height();

    path.lineTo(QPoint{(int)freq_x_pos, zone.height() / 2});
    path.lineTo(QPoint{(int)freq2_x_pos, (int)stopband_y});

    painter->drawPath(path);
}
