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


QT_BEGIN_NAMESPACE
namespace Ui { class FaderSendMtx; }
QT_END_NAMESPACE

class FaderSendMtx : public QWidget {
Q_OBJECT

public:
    explicit FaderSendMtx(QWidget *parent = nullptr);
    ~FaderSendMtx() override;

private:
    Ui::FaderSendMtx *ui;
};


#endif //FADERSENDMTX_H
