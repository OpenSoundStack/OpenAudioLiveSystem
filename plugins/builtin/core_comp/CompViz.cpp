// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CompViz.h"

CompViz::CompViz(QWidget *parent) : QWidget(parent) {
    setFixedSize(500, 500);

    m_threshold_db = 0.0f;
    m_ratio = 1.0f;

    m_hdl_thresh = {};
    m_hdl_thresh.pos_x_db = m_threshold_db;
    m_hdl_thresh.pos_y_db = m_threshold_db;
    m_hdl_thresh.hdl_color = 0xCF1F1F;
    m_hdl_thresh.hovered = false;
    m_hdl_thresh.pressed = false;
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
    draw_grid(painter, zone, CompConfig::comp_depth_db, CompConfig::comp_depth_db, 5);
    draw_comp_curve(painter, zone, m_threshold_db, m_ratio);
    draw_handle(painter, zone, m_hdl_thresh);

    delete painter;
}


void CompViz::draw_comp_curve(QPainter *painter, const QRect &zone, float thresh, float ratio) {
    float threshold_x = (1.0f - (-thresh / CompConfig::comp_depth_db)) * zone.height();
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

void CompViz::draw_handle(QPainter *painter, const QRect &zone, const CompHandleData &handle) {
    float x_pos = zone.width() + (handle.pos_x_db / CompConfig::comp_depth_db) * zone.width();
    float y_pos = (handle.pos_y_db / CompConfig::comp_depth_db) * zone.height();

    QPoint hdl_point{(int)x_pos, (int)y_pos};
    QPainterPath hdl_ellipse{hdl_point};
    hdl_ellipse.addEllipse(hdl_point, 10, 10);

    QBrush brush{handle.hdl_color};
    if (handle.hovered || handle.pressed) {
        brush = QBrush{ThemeColors::handle_selected_color};
    }

    painter->fillPath(hdl_ellipse, brush);
}

void CompViz::set_threshold(float thresh) {
    m_threshold_db = thresh;
    update();
}

void CompViz::set_ratio(float ratio) {
    m_ratio = ratio;
    update();
}
