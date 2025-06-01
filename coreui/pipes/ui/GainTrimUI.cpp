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

#include "GainTrimUI.h"
#include "ui_GainTrimUI.h"


GainTrimUI::GainTrimUI(QWidget *parent) :
    QWidget(parent), ui(new Ui::GainTrimUI) {
    ui->setupUi(this);

    connect(ui->pot_gain, &QDial::valueChanged, this, [this](int value) {
        ui->label_gain->setText(QString::asprintf("%.2f dB", (float)value / 10.0f));
        trigger_value_changed();
    });

    connect(ui->pot_trim, &QDial::valueChanged, this, [this](int value) {
        ui->label_trim->setText(QString::asprintf("%.2f dB", (float)value / 10.0f));
        trigger_value_changed();
    });
}

GainTrimUI::~GainTrimUI() {
    delete ui;
}

void GainTrimUI::trigger_value_changed() {
    emit values_changed(
        ui->pot_gain->value() / 10.0f,
        ui->pot_trim->value() / 10.0f
    );
}

