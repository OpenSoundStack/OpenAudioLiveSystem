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

#include "PipeElemLPF.h"

#include "../core/PipeDesc.h"

PipeElemLPF::PipeElemLPF(AudioRouter* router, float cutoff) : PipeElemDesc(router) {
    m_cutoff = cutoff;
}

void PipeElemLPF::set_cutoff(float cutoff) {
    m_cutoff = cutoff;
}

void PipeElemLPF::render_elem(QRect zone, QPainter *painter) {
    constexpr int stroke_color = 0xB467F0;
    constexpr int fill_color = 0xE3CBF5;

    draw_background(painter, zone);

    float x_pos = freq_to_log_scale(m_cutoff) * zone.width();

    QPoint start_point = QPoint{1, zone.height() / 2};
    QPoint cutoff_point = QPoint{(int)x_pos, zone.height() / 2};
    QPoint min_inf_db_point = QPoint{(int)(cutoff_point.x() + 0.2f * zone.width()), zone.height() + 3};
    QPoint end_point = QPoint{1, zone.height() + 3};

    QPainterPath path = QPainterPath{};
    path.moveTo(start_point);
    path.lineTo(cutoff_point);
    path.lineTo(min_inf_db_point);

    QPen pen = painter->pen();
    pen.setColor(QColor{stroke_color});
    pen.setWidth(2);

    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(pen);
    painter->drawPath(path);

    path.lineTo(end_point);
    painter->fillPath(path, QColor{fill_color});

    draw_frame(painter, zone);
}
