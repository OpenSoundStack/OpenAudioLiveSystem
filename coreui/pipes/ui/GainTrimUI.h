// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef GAINTRIMUI_H
#define GAINTRIMUI_H

#include <QWidget>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class GainTrimUI; }
QT_END_NAMESPACE

class GainTrimUI : public QWidget {
Q_OBJECT

public:
    explicit GainTrimUI(QWidget *parent = nullptr);
    ~GainTrimUI() override;

signals:
    void values_changed(float gain, float trim);

private:
    void trigger_value_changed();

    Ui::GainTrimUI *ui;
};


#endif //GAINTRIMUI_H
