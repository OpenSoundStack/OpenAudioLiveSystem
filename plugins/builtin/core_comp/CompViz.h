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

#include "plugins/loader/ui/GenericVizHandle.h"
#include "plugins/loader/ui/theme.h"

namespace CompConfig {
    constexpr float comp_depth_db = 40.0f;
}

struct CompHandleData : public GenericHandleData {
    float pos_x_db;
    float pos_y_db;
};

class CompViz : public QWidget {

    Q_OBJECT

public:
    CompViz(QWidget* parent = nullptr);
    virtual ~CompViz() = default;

    void set_threshold(float thresh);
    void set_ratio(float ratio);

    static void draw_comp_curve(QPainter* painter, const QRect& zone, float thresh, float ratio);

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void comp_changed(float thresh, float ratio);

private:
    void draw_grid(QPainter* painter, const QRect &zone, float width_db, float height_db, float step_db);
    void draw_handle(QPainter* painter, const QRect& zone, const CompHandleData& handle);

    float m_threshold_db;
    float m_ratio;
    float m_gain;

    CompHandleData m_hdl_thresh;
};



#endif //COMPVIZ_H
