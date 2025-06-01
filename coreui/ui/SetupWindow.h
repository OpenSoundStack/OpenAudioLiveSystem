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

#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QWidget>
#include <QMessageBox>

#include "../core/ShowManager.h"
#include "PipeVisualizer.h"
#include "SignalWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SetupWindow; }
QT_END_NAMESPACE

class SetupWindow : public QWidget {
Q_OBJECT

public:
    explicit SetupWindow(ShowManager* sm, SignalWindow* sw, QWidget *parent = nullptr);
    ~SetupWindow() override;

    void reset_pipe_wizard();
    void setup_add_pipe_page();
private:
    std::optional<PipeDesc*> desc_from_template_combobox();

    Ui::SetupWindow *ui;
    ShowManager* m_sm;

    PipeVisualizer* m_pipe_wiard_viz;
    std::optional<QWidget*> m_current_control;
};


#endif //SETUPWINDOW_H
