// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef COMPVIZ_H
#define COMPVIZ_H

#include <iostream>

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include "plugins/loader/ui/theme.h"

class CompViz : public QWidget {

    Q_OBJECT

public:
    CompViz(QWidget* parent = nullptr);
    virtual ~CompViz() = default;

    static void draw_comp_curve(const QRect& zone, QPainter* painter, float thresh, float ratio);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void draw_grid(QPainter* painter, const QRect &zone, float width_db, float height_db, float step_db);

    float m_threshold_db;
    float m_ratio;
    float m_gain;
};



#endif //COMPVIZ_H
