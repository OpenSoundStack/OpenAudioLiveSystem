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

#include "SignalWindow.h"
#include "ui_SignalWindow.h"

SignalWindow::SignalWindow(QWidget *parent) :
    QWidget(parent), ui(new Ui::SignalWindow) {
    ui->setupUi(this);
}

SignalWindow::~SignalWindow() {
    delete ui;
}

void SignalWindow::set_page_content(const QList<PipeVisualizer *>& pipes) {
    assert(pipes.size() <= 16 && "A page can hold a maximum of 16 pipes.");

    // Remove old pipes viz from current page
    for (auto cp : m_current_page) {
        ui->pipes_container->removeWidget(cp);
    }

    // Show new page
    m_current_page = pipes;

    int index = 0;
    for (auto p : m_current_page) {
        ui->pipes_container->insertWidget(index, p);
        index++;
    }
}

