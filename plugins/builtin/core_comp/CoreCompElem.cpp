// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompElem.h"

#include <QPainterPath>

CoreCompElem::CoreCompElem(AudioRouter *router) : PipeElemDesc(router) {
    auto* compui = new CoreComp_UI();
    m_controls = compui;

    m_threshold = 0.0f;
    m_ratio = 1.0f;

    connect(compui, &CoreComp_UI::comp_changed, this, [this](float thresh, float ratio, float gain) {
        m_threshold = thresh;
        m_ratio = ratio;

        update();
    });
}

void CoreCompElem::render_elem(QRect zone, QPainter *painter) {
    QRect transfer_zone = zone;
    transfer_zone.setWidth(zone.height() * 0.90f);
    transfer_zone.setHeight(zone.height() * 0.90f);

    transfer_zone.translate(
        zone.width() * 0.05f,
        (zone.height() - transfer_zone.height()) / 2
    );

    draw_background(painter, zone);

    CompViz::draw_comp_curve(painter, transfer_zone, m_threshold, m_ratio);

    draw_frame(painter, transfer_zone);
    draw_frame(painter, zone);
}
