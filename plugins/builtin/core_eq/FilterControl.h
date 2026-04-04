// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
