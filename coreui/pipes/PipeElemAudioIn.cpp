// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "PipeElemAudioIn.h"

PipeElemAudioIn::PipeElemAudioIn(AudioRouter* router) : PipeElemDesc(router) {
    setFixedHeight(60);

    auto* fx_ui = new GainTrimUI();
    m_controls = fx_ui;
    m_control_data = std::make_shared<GenericElemControlData<GainTrim>>(GainTrim{1.0f, 1.0f, false});
    m_gt_db = GainTrim{0, 0, false};

    register_control(1, m_control_data);

    m_router = router;

    connect(fx_ui, &GainTrimUI::values_changed, this, [this](float gain, float trim, bool en_48v) {
        m_control_data->set_data({
            get_lin(gain), get_lin(trim), en_48v
        });

        m_gt_db = {gain, trim, en_48v};
        update();

        send_control_packets();
    });

    m_flags = ElemFlags::ELEM_IS_SIMPLE_IO;
}

void PipeElemAudioIn::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);

    // Two zones separation
    QRect gain_rect = zone;
    gain_rect.setWidth(zone.width() / 2);
    gain_rect.setHeight(zone.height() * 0.7f);

    QRect trim_rect = zone;
    trim_rect.setWidth(zone.width() / 2);
    trim_rect.setHeight(zone.height() * 0.7f);
    trim_rect.moveTo(QPoint{zone.width() / 2, zone.topLeft().y()});

    QRect phantom_rect = zone;
    phantom_rect.setHeight(zone.height() * 0.3f);
    phantom_rect.moveTo(gain_rect.bottomLeft());

    GainTrim& values = m_control_data->get_data();
    QString gain_text = QString::asprintf("GAIN\n%.1f dB", m_gt_db.gain);
    QString trim_text = QString::asprintf("TRIM\n%.1f dB", m_gt_db.trim);

    painter->drawText(gain_rect, Qt::AlignCenter, gain_text);
    painter->drawText(trim_rect, Qt::AlignCenter, trim_text);

    QPen pen = painter->pen();
    if (m_gt_db.phantom_en) {
        painter->fillRect(phantom_rect, Qt::red);
        pen.setColor(Qt::white);
    } else {
        painter->fillRect(phantom_rect, Qt::darkGray);
        pen.setColor(Qt::black);
    }

    painter->setPen(pen);
    painter->drawText(phantom_rect, Qt::AlignCenter, "+48V");

    draw_frame(painter, zone);
}

float PipeElemAudioIn::get_db(float lin_val) {
    return 20.0f * std::log10(lin_val);
}

float PipeElemAudioIn::get_lin(float db_val) {
    return std::pow(10, db_val / 20.0f);
}

