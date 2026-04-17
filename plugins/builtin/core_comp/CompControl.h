// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

//
// Created by mathis on 17/04/2026.
//

#ifndef COMPCONTROL_H
#define COMPCONTROL_H

#include <QWidget>
#include <QDial>
#include <QDoubleSpinBox>

#include "CompViz.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CompControl; }
QT_END_NAMESPACE

class CompControl : public QWidget {
Q_OBJECT

public:
    explicit CompControl(QWidget *parent = nullptr);
    ~CompControl() override;

    void set_threshold(float thresh);
    void set_ratio(float ratio);
    void set_gain(float gain);

signals:
    void param_changed(float thresh, float ratio, float gain);
    void time_params_changed(int attack, int release, int hold);

private:
    float normalize_dial(int value, QDial* dial);
    void map_to_dial(double value, QDial* dial, float min, float max);

    void load_defaults();

    void emit_params();
    void emit_time_params();

    Ui::CompControl *ui;
};


#endif //COMPCONTROL_H
