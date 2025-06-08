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

#include "PipeElemAudioIn.h"

PipeElemAudioIn::PipeElemAudioIn(AudioRouter* router) : PipeElemDesc(router) {
    setFixedHeight(60);

    GainTrimUI* fx_ui = new GainTrimUI();
    m_controls = fx_ui;
    m_control_data = std::make_shared<GenericElemControlData<GainTrim>>(GainTrim{1.0f, 1.0f});

    register_control(1, m_control_data);

    m_router = router;

    connect(fx_ui, &GainTrimUI::values_changed, this, [this](float gain, float trim) {
        m_control_data->set_data({
            get_lin(gain), get_lin(trim)
        });
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

    QRect trim_rect = zone;
    trim_rect.setWidth(zone.width() / 2);
    trim_rect.moveTo(QPoint{zone.width() / 2, zone.topLeft().y()});

    GainTrim& values = m_control_data->get_data();
    float gain_val_db = get_db(values.gain);
    float trim_val_db = get_db(values.trim);

    QString gain_text = QString::asprintf("GAIN\n%.1f dB", gain_val_db);
    QString trim_text = QString::asprintf("TRIM\n%.1f dB", trim_val_db);

    painter->drawText(gain_rect, Qt::AlignCenter, gain_text);
    painter->drawText(trim_rect, Qt::AlignCenter, trim_text);

    draw_frame(painter, zone);
}

float PipeElemAudioIn::get_db(float lin_val) {
    return 20.0f * log10(lin_val);
}

float PipeElemAudioIn::get_lin(float db_val) {
    return pow(10, db_val / 20.0f);
}

