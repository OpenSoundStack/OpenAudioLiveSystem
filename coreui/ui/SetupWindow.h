// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QWidget>
#include <QMessageBox>

#include "../core/ShowManager.h"
#include "PipeVisualizer.h"
#include "SignalWindow.h"
#include "RoutingPage.h"

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

    RoutingPage* m_routing_page = nullptr;
    int m_routing_page_index = -1;
};


#endif //SETUPWINDOW_H
