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

struct HandleData {
    float fc;
    float gain;

    bool pressed;
    bool hovered;

    uint32_t hdl_color;
};

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

    virtual void set_cutoff(float fc, int handle_idx);
    virtual void set_gain(float gain, int handle_idx);
    virtual void set_Q(float Q, int handle_idx);

    void draw_curve(QPainter* painter, QRect zone, const std::vector<std::pair<float, float>>& curve);
    std::vector<QPointF> calc_curve(const std::vector<std::pair<float, float>>& curve);

    void add_handle(float fc, float gain, uint32_t color = 0xFFFFFF);
    QPoint get_handle_loc(int index, QRect zone);

signals:
void handle_moved(float fc, float gain, int index);

protected:
    virtual void draw_approx_filter(QPainter* painter, QRect zone);
    virtual void calc_filter_mag();
    std::vector<std::pair<float, float>> m_filter_mag;

    std::vector<HandleData> m_handles;

private:
    void draw_grid(QPainter* painter, QRect zone);
    void draw_filter_mag(QPainter* painter, QRect zone);
    void draw_handle(int index, QPainter* painter, QRect zone);

    float gain_to_ycoord(float dbgain);

    Ui::FilterEditBase *ui;
};


#endif //FILTEREDITBASE_H
