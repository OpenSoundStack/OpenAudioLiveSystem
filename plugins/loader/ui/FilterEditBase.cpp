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

#include "FilterEditBase.h"

#include "ui_FilterEditBase.h"


FilterEditBase::FilterEditBase(QWidget *parent) :
    QWidget(parent), ui(new Ui::FilterEditBase) {
    ui->setupUi(this);

    setMouseTracking(true);
}

FilterEditBase::~FilterEditBase() {
    delete ui;
}

void FilterEditBase::set_cutoff(float fc, int handle_idx) {

}

void FilterEditBase::set_gain(float gain, int handle_idx) {

}

void FilterEditBase::paintEvent(QPaintEvent *event) {
    constexpr int bg_color = 0x1E1E1E;
    constexpr int stroke_color = 0xB467F0;

    auto* painter = new QPainter{this};
    painter->setRenderHint(QPainter::Antialiasing);

    QRect zone = event->rect();

    painter->fillRect(zone, QBrush{bg_color});
    draw_grid(painter, zone);
    draw_filter_mag(painter, zone);

    // STROKE CONFIG
    QPen pen = painter->pen();
    pen.setWidth(3);
    pen.setColor(stroke_color);
    painter->setPen(pen);

    // Drawing strokes
    draw_filter_mag(painter, zone);
    draw_approx_filter(painter, zone);

    for (int i = 0; i < m_handles.size(); i++) {
        draw_handle(i, painter, zone);
    }

    delete painter;
}

void FilterEditBase::draw_grid(QPainter *painter, QRect zone) {
    constexpr int strong_lines = 0x4E4E4E;
    constexpr int light_lines = 0x2E2E2E;

    QPen pen = painter->pen();
    pen.setWidth(2);
    pen.setColor(strong_lines);
    painter->setPen(pen);

    painter->drawLine(0, zone.height() / 2, zone.width(), zone.height() / 2);

    pen.setWidth(1);
    painter->setPen(pen);

    // Draw semilog lines
    for (int base = 10; base <= 10000; base *= 10) {
        pen.setWidth(2); // Highligh first elem of the decade
        pen.setColor(strong_lines);
        painter->setPen(pen);

        if (IS_BETWEEN(20, base, 20000)) {
            int text_pos = zone.width() * freq_to_log_scale(base);
            QPoint text_point = QPoint{text_pos, zone.height() / 2};
            text_point += QPoint{5, -5};

            painter->drawText(text_point, QString::number(base));
        }

        for (int i = 1; i <= 9; i++) {
            if (IS_BETWEEN(20, base * i, 20000)) {
                int line_pos = zone.width() * freq_to_log_scale(base * i);
                painter->drawLine(line_pos, 0, line_pos, zone.height());
            }

            pen.setWidth(1);
            pen.setColor(light_lines);
            painter->setPen(pen); // Reset pen
        }
    }

    pen.setWidth(1);
    pen.setColor(light_lines);
    painter->setPen(pen); // Reset pen

    // Draw dB lines
    for (int i = 0; i < 6; i++) {
        int y = zone.height() * i / 6;
        painter->drawLine(0, y, zone.width(), y);
    }
}

void FilterEditBase::draw_filter_mag(QPainter *painter, QRect zone) {
    draw_curve(painter, zone, m_filter_mag);
}

void FilterEditBase::draw_curve(QPainter *painter, QRect zone, const std::vector<std::pair<float, float> > &curve) {
    QPainterPath path{};
    int index = 0;

    for (auto& value_pair : curve) {
        if (!IS_BETWEEN(20, value_pair.first, 20000)) {
            continue;
        }

        float frequency_xpos = freq_to_log_scale(value_pair.first) * zone.width();

        float db_mag = 20.0f * log10(value_pair.second);

        float mag_y_mapped = 0; // Hide stroke behind boundaries
        if (db_mag != -INFINITY) {
            mag_y_mapped = (db_mag + 18.0f) / 36.0f; // Getting db mapped between 0 and 1
            mag_y_mapped *= zone.height();
        }

        if (index == 0) {
            path.moveTo(QPointF{frequency_xpos, (zone.height() - mag_y_mapped)});
        } else {
            path.lineTo(QPointF{frequency_xpos, (zone.height() - mag_y_mapped)});
        }

        index++;
    }

    painter->drawPath(path);
}

void FilterEditBase::draw_handle(int index, QPainter *painter, QRect zone) {
    constexpr int handle_selected_color = 0x03a5fc;

    HandleData& hdl = m_handles[index];

    QPoint point = get_handle_loc(index, zone);
    QPoint fc_text_point = QPoint{10, 10};
    QRect fc_text_rect{fc_text_point, QPoint{150, 50}};

    QPainterPath path{point};
    path.addEllipse(point, 10, 10);

    QBrush brush{Qt::white};
    if (hdl.hovered || hdl.pressed) {
        brush = QBrush{handle_selected_color};
    }

    painter->fillPath(path, brush);

    QFont font = painter->font();
    font.setPointSize(font.pointSize() * 1.5f);
    painter->setFont(font);

    //painter->drawText(fc_text_rect, QString::asprintf("%.0f Hz", m_fc));
}

QPoint FilterEditBase::get_handle_loc(int index, QRect zone) {
    HandleData& hdl = m_handles[index];

    float fc_x_pos = freq_to_log_scale(hdl.fc) * zone.width();
    float y_pos = zone.height() * gain_to_ycoord(hdl.gain);
    QPoint point{(int)fc_x_pos, (int)y_pos};

    return point;
}

float FilterEditBase::gain_to_ycoord(float dbgain) {
    // Drawn scale is 36 dB wide, -18 dB to +18 dB
    return (18.0f - dbgain) / 36.0f;
}

void FilterEditBase::calc_filter_mag() {

}

void FilterEditBase::mouseMoveEvent(QMouseEvent *event) {
    QRect zone = rect();

    QPointF curpos = event->position();

    for (int i = 0; i < m_handles.size(); i++) {
        QPoint handle_pos = get_handle_loc(i, zone);
        HandleData& hdl = m_handles[i];

        float dist2 = pow(curpos.x() - handle_pos.x(), 2) + pow(curpos.y() - handle_pos.y(), 2);

        if (dist2 < 100) {
            hdl.hovered = true;
            update();
        } else {
            hdl.hovered = false;
            update();
        }

        if (hdl.pressed) {
            float freq = log_scale_to_freq((float)curpos.x() / (float)zone.width());
            int ceiled_freq = round(freq);

            float gain = (-(curpos.y() / zone.height()) + 0.5f) * 36.0f;
            if (gain > 18) {
                gain = 18;
            } else if (gain < -18) {
                gain = -18;
            }

            if (IS_BETWEEN(20, ceiled_freq, 20000)) {
                set_cutoff(ceiled_freq, i);
            } else if (ceiled_freq > 20000) {
                set_cutoff(20000, i);
            }

            set_gain(gain, i);

            emit handle_moved(hdl.fc, hdl.gain, i);
        }
    }
}

void FilterEditBase::mousePressEvent(QMouseEvent *event) {
    for (auto& hdl : m_handles) {
        if (hdl.hovered) {
            hdl.pressed = true;
            break; // Avoid grabbing multiple points
        }
    }
}

void FilterEditBase::mouseReleaseEvent(QMouseEvent *event) {
    for (auto& hdl : m_handles) {
        hdl.pressed = false;
    }
}

void FilterEditBase::draw_approx_filter(QPainter *painter, QRect zone) {

}

void FilterEditBase::add_handle(float fc, float gain) {
    m_handles.emplace_back(fc, gain);
}
