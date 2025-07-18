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

//
// Created by mathis on 05/06/25.
//

#ifndef FADERSENDMTX_H
#define FADERSENDMTX_H

#include <QWidget>
#include <QSlider>

#include "coreui/ui/Fader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FaderSendMtx; }
QT_END_NAMESPACE

class FaderSendMtx : public QWidget {
Q_OBJECT

public:
    explicit FaderSendMtx(QWidget *parent = nullptr);
    ~FaderSendMtx() override;

    void add_fader(QString name, uint8_t channel, uint16_t host);

signals:
    void fader_value_changed(uint8_t channel, uint16_t host, float value);

private:
    Ui::FaderSendMtx *ui;

    QList<Fader*> m_faders;
};


#endif //FADERSENDMTX_H
