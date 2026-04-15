// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef FILTEREDITBASE_H
#define FILTEREDITBASE_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

#include <vector>
#include <iostream>

#include "plugins/loader/ui/VizUtils.h"
#include "plugins/loader/ui/theme.h"
#include "plugins/loader/ui/GenericVizHandle.h"

struct FilterHandleData : public GenericHandleData {
    float fc;
    float gain;
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

    std::vector<FilterHandleData> m_handles;

private:
    void draw_grid(QPainter* painter, QRect zone);
    void draw_filter_mag(QPainter* painter, QRect zone);
    void draw_handle(int index, QPainter* painter, QRect zone);

    float gain_to_ycoord(float dbgain);

    Ui::FilterEditBase *ui;
};


#endif //FILTEREDITBASE_H
