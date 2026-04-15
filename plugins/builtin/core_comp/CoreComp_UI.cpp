// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreComp_UI.h"

CoreComp_UI::CoreComp_UI(QWidget *parent) : QWidget(parent) {
    m_ui_layout = new QGridLayout(this);

    m_comp_viz = new CompViz();
    m_ui_layout->addWidget(m_comp_viz, 0, 0);

    connect(m_comp_viz, &CompViz::comp_changed, this, [this](float thresh, float ratio) {
        emit comp_changed(thresh, ratio, 0.0f);
    });
}
