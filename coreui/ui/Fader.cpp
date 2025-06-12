//  This file is part of the Open Audio Live System project, a live audio environment
//  Copyright (c) 2025 - Mathis DELGADO
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, version 3 of the License.
//
//  This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.

// You may need to build the project (run Qt uic code generator) to get "ui_Fader.h" resolved

#include "Fader.h"
#include "ui_Fader.h"


Fader::Fader(QWidget *parent) :
        QWidget(parent), ui(new Ui::Fader) {
    ui->setupUi(this);

    connect(ui->fader, &QSlider::valueChanged, this, [this](int value) {
        float log_scale = 0.0f; // Linear value scaled on a log scale

        if (value <= 2500.0f) {
            // -inf dB => +0 dB range
            float lin_value = 1.0 + ((float)(2500.0f - value) / 2500.0f);
            log_scale = 1.0f - map_to_log_scale(lin_value, 1.0f, 2.0f);
        } else {
            // +0 dB => +10 dB range
            float lin_value = value / 2500.0f;
            float range_ratio = (float)ui->fader->maximum() / 2500.0f;

            // 3.2 is the linear value for +10 dB
            // I subtracted 1 to 3.2 to compensate for + 1 at the end, which guarantee that we are working
            // in the 0 => 10 dB range, and not the -inf => +10 dB range
            log_scale = map_to_log_scale(lin_value, 1.0f, range_ratio) * 2.2f + 1.0f;
        }

        float db_value = 20.0f * log10(log_scale);
        ui->level_disp->setText(QString::asprintf("%.2f dB", db_value));

        emit value_changed((float)log_scale);
    });
}

Fader::~Fader() {
    delete ui;
}

void Fader::set_fader_name(QString name) {
    ui->label->setText(name);
}
