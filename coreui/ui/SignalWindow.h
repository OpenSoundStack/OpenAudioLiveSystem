// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
