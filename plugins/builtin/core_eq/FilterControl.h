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

#ifndef FILTERCONTROL_H
#define FILTERCONTROL_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class FilterControl; }
QT_END_NAMESPACE

class FilterControl : public QWidget {
Q_OBJECT

public:
    explicit FilterControl(uint32_t control_color = 0xFFFFFF, QWidget *parent = nullptr);
    ~FilterControl() override;

    void set_freq(int new_freq);
    void set_gain(float gain_db);
    void set_Q(float Q);

signals:
    void filter_changed(float freq, float gain_db, float Q);
private:
    Ui::FilterControl *ui;
};


#endif //FILTERCONTROL_H
