// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef ROUTING_GRID_WIDGET_H
#define ROUTING_GRID_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QMouseEvent>

#include <OpenAudioNetwork/common/AudioRouter.h>

#include "plugins/loader/ui/theme.h"

namespace RoutingWidgetConfig {
    constexpr int cell_width_height = 25;
    constexpr int heading_length = 150;
    constexpr int heading_margin = 10;
}

struct RoutingGridMetrics {
    int zone_max_cols;
    int zone_max_rows;
    int x_max;
    int y_max;
};

enum class RoutingCellState {
    ROUTE_VALID,
    ROUTE_PENDING
};

enum class RoutingMode {
    ROUTING_SOURCE,
    ROUTING_SINK
};

struct RoutingCell {
    RouteNode from;
    RouteNode to;
    QPoint grid_pos;
    RoutingCellState state;
};

struct RoutingNodeMeta {
    QString name;
    RouteNode rnode;
};

class RoutingGridWidget : public QWidget {
    Q_OBJECT

public:
    RoutingGridWidget(QWidget* parent = nullptr);
    ~RoutingGridWidget() override = default;

    void validate_route(const RouteNode& from, const RouteNode& to);

    void add_node(QString node_name, RoutingMode mode, const RouteNode& routing_node);
signals:
    void try_create_route(const RouteNode& from, const RouteNode& to);
    void try_move_route(const RouteNode& from, const RouteNode& old_destination, const RouteNode& new_destination);

protected:
    void paintEvent(QPaintEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    RoutingGridMetrics compute_metrics_for_zone(const QRect& zone);
    void update_metrics(const QRect& zone);

    void draw_grid(QPainter* painter) const;
    void draw_text(QPainter* painter) const;
    void draw_dots(QPainter* painter) const;

    bool route_exists_on_row(int row);
    void add_pending_cell(QPoint grid_pos);
    void move_cell_on_row(int row, int new_col);

    QPoint get_grid_pos(const QPoint& local_pos) const;

    RoutingGridMetrics m_grid_metrics;

    bool m_cursor_hovering_grid;
    bool m_mouse_pressed;

    QPoint m_cursor_press_pos;
    QPoint m_cursor_grid_pos;

    std::vector<RoutingCell> m_grid_state;

    std::vector<RoutingNodeMeta> m_columns;
    std::vector<RoutingNodeMeta> m_rows;
};


#endif // ROUTING_GRID_WIDGET_H
