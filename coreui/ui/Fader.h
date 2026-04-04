// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_FADER_H
#define OALIVESYSTEM_FADER_H

#include <QWidget>
#include "plugins/loader/ui/VizUtils.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Fader; }
QT_END_NAMESPACE

class Fader : public QWidget {
Q_OBJECT

public:
    explicit Fader(QWidget *parent = nullptr);
    ~Fader() override;

    void set_fader_name(QString name);

signals:
    void value_changed(float value);

private:
    Ui::Fader *ui;
};


#endif //OALIVESYSTEM_FADER_H
