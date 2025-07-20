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

#include "CoreEqElem.h"

CoreEqElem::CoreEqElem(AudioRouter *router) : PipeElemDesc(router) {
    m_main_ui = new CoreEQ_UI{};
    m_controls = m_main_ui;

    m_control_ui = m_main_ui->get_control_ui();

    init_filters();
    m_control_ui->calc_filter_mag(); // Init EQ curve

    connect(m_main_ui, &CoreEQ_UI::filter_changed, this, [this](float fc, float gain, float Q, int index) {
        FilterParams& data = m_filters[index]->get_data();
        data.fc = fc;
        data.gain = gain;
        data.Q = Q;

        m_filters[index]->set_data(data);
        send_control_packets();

        // Update thumbnail curve
        update_curve();
    });
}

void CoreEqElem::render_elem(QRect zone, QPainter *painter) {
    constexpr int stroke_color = 0xB467F0;

    draw_background(painter, zone);

    // Draw EQ stroke
    QPainterPath stroke_path{};
    stroke_path.moveTo(m_eq_curve[0]);

    for (auto& p : m_eq_curve) {
        auto point = QPointF{zone.width() * p.x(), zone.height() * p.y()};
        stroke_path.lineTo(point);
    }

    QPen pen = painter->pen();
    pen.setColor(stroke_color);
    pen.setWidth(2);
    painter->setPen(pen);

    painter->drawPath(stroke_path);

    // Draw EQ points

    pen.setWidth(4);
    for (int i = 0; i < 6; i++) {
        QPoint hdl_loc = m_control_ui->get_handle_loc(i, zone);

        pen.setColor(handle_colors[i]);
        painter->setPen(pen);

        painter->drawPoint(hdl_loc);
    }

    draw_frame(painter, zone);
}

void CoreEqElem::init_filters() {
    for (int i = 0; i < 6; i++) {
        m_filters[i] = std::make_shared<GenericElemControlData<FilterParams>>(FilterParams{
            .fc = default_frequencies[i],
            .gain = 0,
            .Q = default_Q
        });

        register_control(i + 1, m_filters[i]);
    }

    update_curve();
}

void CoreEqElem::update_curve() {
    m_eq_curve = std::move(m_control_ui->get_eq_curve());
    update();
}
