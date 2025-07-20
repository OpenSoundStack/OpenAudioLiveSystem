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

#include "FilterControl.h"
#include "ui_FilterControl.h"


FilterControl::FilterControl(uint32_t control_color, QWidget *parent) :
    QWidget(parent), ui(new Ui::FilterControl) {
    ui->setupUi(this);


    ui->colored_widget->setStyleSheet(QString::asprintf("QWidget { background-color: #%06X }", control_color));

    connect(ui->sp_freq, &QSpinBox::valueChanged, this, [this](int value) {
        emit filter_changed(value, ui->sp_gain->value(), ui->sp_q->value());
    });

    connect(ui->sp_gain, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        emit filter_changed(ui->sp_freq->value(), value, ui->sp_q->value());
    });

    connect(ui->sp_q, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        emit filter_changed(ui->sp_freq->value(), ui->sp_gain->value(), value);
    });
}

FilterControl::~FilterControl() {
    delete ui;
}

void FilterControl::set_freq(int new_freq) {
    ui->sp_freq->setValue(new_freq);
}

void FilterControl::set_gain(float gain_db) {
    ui->sp_gain->setValue(gain_db);
}

void FilterControl::set_Q(float Q) {
    ui->sp_q->setValue(Q);
}

