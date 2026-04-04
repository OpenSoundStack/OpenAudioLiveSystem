// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef COREEQ_UI_H
#define COREEQ_UI_H

#include <array>

#include <QWidget>
#include <QGridLayout>

#include "FilterControl.h"
#include "CoreEqControlUI.h"

#include "common.h"

class CoreEQ_UI : public QWidget {

    Q_OBJECT

public:
    CoreEQ_UI(QWidget* parent = nullptr);
    ~CoreEQ_UI() override = default;

    CoreEqControlUI* get_control_ui();

signals:
    void filter_changed(float freq, float gain, float Q, int index);
private:
    QGridLayout* m_ui_layout;

    CoreEqControlUI* m_control_ui;
    std::array<FilterControl*, 6> m_filter_controls;
};



#endif //COREEQ_UI_H
