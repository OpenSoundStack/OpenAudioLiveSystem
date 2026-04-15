// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CompViz.h"

CompViz::CompViz(QWidget *parent) : QWidget(parent) {
    setFixedSize(500, 500);

    m_threshold_db = 0.0f;
    m_ratio = 1.0f;
}


void CompViz::draw_grid(QPainter *painter, const QRect &zone, float width_db, float height_db, float step_db) {
    painter->fillRect(zone, ThemeColors::bg_color);

    int line_count_v = static_cast<int>(width_db / step_db);
    int line_count_h = static_cast<int>(height_db / step_db);

    QPen pen = painter->pen();
    pen.setColor(ThemeColors::grid_color_light);
    pen.setWidth(1);
    painter->setPen(pen);

    for (int i = 0; i < line_count_v; i++) {
        float line_level = (step_db * i) / width_db;
        float line_y = line_level * zone.width();

        painter->drawLine(
            0, (int)line_y,
            zone.width(), (int)line_y
        );
    }

    for (int i = 0; i < line_count_h; i++) {
        float line_level = (step_db * i) / height_db;
        float line_x = line_level * (float)zone.height();

        painter->drawLine(
            (int)line_x, 0,
            (int)line_x, zone.height()
        );
    }
}

void CompViz::paintEvent(QPaintEvent *event) {
    auto* painter = new QPainter{this};
    painter->setRenderHint(QPainter::Antialiasing);

    QRect zone = event->rect();
    draw_grid(painter, zone, 40, 40, 5);
    draw_comp_curve(zone, painter, m_threshold_db, m_ratio);

    delete painter;
}


void CompViz::draw_comp_curve(const QRect &zone, QPainter *painter, float thresh, float ratio) {
    // Going from -40 dB threshold to 0dB
    float comp_range = 40.0f;

    float threshold_x = (1.0f - (-thresh / comp_range)) * zone.height();
    float compressed_y = ((zone.height() - threshold_x) / ratio) + threshold_x;

    // Draw Transfer function
    QPainterPath stroke_path{};
    stroke_path.moveTo(0, 0);
    stroke_path.lineTo(
        threshold_x,
        threshold_x
    );

    stroke_path.lineTo(
        zone.height(),
        compressed_y
    );

    QPen pen = painter->pen();
    pen.setColor(ThemeColors::stroke_color);
    pen.setWidth(2);
    painter->setPen(pen);

    auto transform = painter->transform();

    painter->translate(zone.bottomLeft());
    painter->scale(1.0f, -1.0f);

    painter->drawPath(stroke_path);

    painter->setTransform(transform);
}
