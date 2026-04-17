// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_CORECOMPUI_H
#define OALIVESYSTEM_CORECOMPUI_H

#include <QWidget>
#include <QHBoxLayout>

#include "CompViz.h"
#include "CompControl.h"

class CoreComp_UI : public QWidget {

    Q_OBJECT

public:
    CoreComp_UI(QWidget* parent = nullptr);
    ~CoreComp_UI() override = default;

signals:
    void comp_changed(float threshold, float ratio, float gain);

private:
    QGridLayout* m_ui_layout;

    CompViz* m_comp_viz;
    CompControl* m_comp_control;
};



#endif //OALIVESYSTEM_CORECOMPUI_H
