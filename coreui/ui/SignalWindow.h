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

#ifndef SIGNALWINDOW_H
#define SIGNALWINDOW_H

#include <QWidget>
#include <QList>

#include "PipeVisualizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SignalWindow; }
QT_END_NAMESPACE

class SignalWindow : public QWidget {
Q_OBJECT

public:
    explicit SignalWindow(QWidget *parent = nullptr);
    ~SignalWindow() override;

    void set_page_content(const QList<PipeVisualizer*>& pipes);
private:
    Ui::SignalWindow *ui;

    QList<PipeVisualizer*> m_current_page;
};


#endif //SIGNALWINDOW_H
