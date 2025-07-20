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

#include "CoreEQ_UI.h"

CoreEQ_UI::CoreEQ_UI(QWidget *parent) : QWidget(parent) {
    m_ui_layout = new QGridLayout(this);

    m_control_ui = new CoreEqControlUI();
    m_ui_layout->addWidget(m_control_ui, 0, 0, 1, 6);

    connect(m_control_ui, &CoreEqControlUI::handle_moved, this, [this](float fc, float gain, int index) {
        m_filter_controls[index]->set_freq(fc);
        m_filter_controls[index]->set_gain(gain);
    });

    for (int i = 0; i < 6; i++) {
        m_filter_controls[i] = new FilterControl(handle_colors[i], this);
        m_filter_controls[i]->set_freq(default_frequencies[i]);
        m_filter_controls[i]->set_Q(default_Q);

        m_ui_layout->addWidget(m_filter_controls[i], 1, i);

        connect(m_filter_controls[i], &FilterControl::filter_changed, this, [this, i](float freq, float gain, float Q) {
            m_control_ui->set_cutoff(freq, i);
            m_control_ui->set_gain(gain, i);
            m_control_ui->set_Q(Q, i);
            m_control_ui->update();

            emit filter_changed(freq, gain, Q, i);
        });
    }
}

CoreEqControlUI *CoreEQ_UI::get_control_ui() {
    return m_control_ui;
}
