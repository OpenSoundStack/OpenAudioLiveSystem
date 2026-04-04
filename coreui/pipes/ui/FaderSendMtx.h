// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
