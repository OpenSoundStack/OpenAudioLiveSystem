// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompElem.h"

#include <QPainterPath>

CoreCompElem::CoreCompElem(AudioRouter *router) : PipeElemDesc(router) {
    m_controls = new CoreComp_UI();

    m_threshold = 0.0f;
    m_ratio = 1.0f;
}

void CoreCompElem::render_elem(QRect zone, QPainter *painter) {
    constexpr int stroke_color = 0xB467F0;
    QRect transfer_zone = zone;
    transfer_zone.setWidth(zone.height() * 0.90f);
    transfer_zone.setHeight(zone.height() * 0.90f);

    transfer_zone.translate(
        zone.width() * 0.05f,
        (zone.height() - transfer_zone.height()) / 2
    );

    draw_background(painter, zone);

    // Going from -40 dB threshold to 0dB
    float comp_range = 40.0f;

    float threshold_x = (1.0f - (-m_threshold / comp_range)) * transfer_zone.height();
    float compressed_y = ((transfer_zone.height() - threshold_x) / m_ratio) + threshold_x;

    // Draw Transfer function
    QPainterPath stroke_path{};
    stroke_path.moveTo(0, 0);
    stroke_path.lineTo(
        threshold_x,
        threshold_x
    );

    stroke_path.lineTo(
        transfer_zone.height(),
        compressed_y
    );

    QPen pen = painter->pen();
    pen.setColor(stroke_color);
    pen.setWidth(2);
    painter->setPen(pen);

    auto transform = painter->transform();

    painter->translate(transfer_zone.bottomLeft());
    painter->scale(1.0f, -1.0f);

    painter->drawPath(stroke_path);

    painter->setTransform(transform);

    draw_frame(painter, transfer_zone);
    draw_frame(painter, zone);
}
