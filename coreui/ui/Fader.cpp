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
}

Fader::~Fader() {
    delete ui;
}

void Fader::set_fader_name(QString name) {
    ui->label->setText(name);
}
