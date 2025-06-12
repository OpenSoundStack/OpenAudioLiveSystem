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

#include "FaderSendMtx.h"
#include "ui_FaderSendMtx.h"


FaderSendMtx::FaderSendMtx(QWidget *parent) :
    QWidget(parent), ui(new Ui::FaderSendMtx) {
    ui->setupUi(this);
}

FaderSendMtx::~FaderSendMtx() {
    delete ui;
}

void FaderSendMtx::add_fader(QString name, uint8_t channel, uint16_t host) {
    Fader* new_fader = new Fader();
    new_fader->set_fader_name(name);

    m_faders.append(new_fader);
    ui->fader_page_container->addWidget(new_fader);

    connect(new_fader, &Fader::value_changed, this, [this, channel, host](float value) {
        emit fader_value_changed(channel, host, value);
    });
}
