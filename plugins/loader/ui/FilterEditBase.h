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

#ifndef FILTEREDITBASE_H
#define FILTEREDITBASE_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

#include <vector>
#include <iostream>

#include "plugins/loader/ui/VizUtils.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FilterEditBase; }
QT_END_NAMESPACE

class FilterEditBase : public QWidget {
Q_OBJECT

public:
    explicit FilterEditBase(QWidget *parent = nullptr);
    ~FilterEditBase() override;

    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void set_cutoff(float fc) = 0;

signals:
void handle_moved(float fc);

protected:
    virtual void draw_approx_filter(QPainter* painter, QRect zone);
    virtual void calc_filter_mag();
    std::vector<std::pair<float, float>> m_filter_mag;

    float m_fc;

private:
    void draw_grid(QPainter* painter, QRect zone);
    void draw_filter_mag(QPainter* painter, QRect zone);
    void draw_handle(QPainter* painter, QRect zone);

    QPoint get_handle_loc(QRect zone);

    Ui::FilterEditBase *ui;

    bool m_handle_hovered;
    bool m_handle_pressed;
};


#endif //FILTEREDITBASE_H
