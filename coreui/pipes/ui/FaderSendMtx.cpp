// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
