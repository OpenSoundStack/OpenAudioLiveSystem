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
