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
