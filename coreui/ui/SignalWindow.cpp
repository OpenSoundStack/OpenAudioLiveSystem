// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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

