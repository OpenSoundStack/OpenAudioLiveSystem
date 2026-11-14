// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "RoutingGridWidget.h"

#include <utility>

RoutingGridWidget::RoutingGridWidget(QWidget* parent) : QWidget(parent) {
    m_cursor_hovering_grid = false;
    m_mouse_pressed = false;

    m_cursor_grid_pos = QPoint{0, 0};
    m_cursor_press_pos = QPoint{0, 0};
    m_grid_metrics = RoutingGridMetrics{0, 0, 0, 0};

    setMouseTracking(true);
}

void RoutingGridWidget::paintEvent(QPaintEvent *event) {
    auto* painter = new QPainter{this};
    QRect r = event->rect();

    update_metrics(r);

    painter->fillRect(r, ThemeColors::bg_color);
    draw_grid(painter);
    draw_text(painter);
    draw_dots(painter);

    event->accept();
    delete painter;
}

void RoutingGridWidget::update_metrics(const QRect& zone) {
    m_grid_metrics = compute_metrics_for_zone(zone);
}

QPoint RoutingGridWidget::get_grid_pos(const QPoint &local_pos) const {
    int x = (local_pos.x() - RoutingWidgetConfig::heading_length) / RoutingWidgetConfig::cell_width_height;
    x = std::min(x, static_cast<int>(m_columns.size()));

    int y = (local_pos.y() - RoutingWidgetConfig::heading_length) / RoutingWidgetConfig::cell_width_height;
    y = std::min(y, static_cast<int>(m_rows.size()));

    QPoint gpoint{};
    gpoint.setX(x);
    gpoint.setY(y);

    return gpoint;
}

void RoutingGridWidget::mouseMoveEvent(QMouseEvent *event) {
    const auto point = event->position();
    m_cursor_hovering_grid = point.x() > RoutingWidgetConfig::heading_length && point.y() > RoutingWidgetConfig::heading_length;
    m_cursor_hovering_grid &= point.x() < m_grid_metrics.x_max && point.y() < m_grid_metrics.y_max;

    if (m_cursor_hovering_grid && !m_mouse_pressed) {
        m_cursor_grid_pos = get_grid_pos(point.toPoint());
    }

    update();
}

void RoutingGridWidget::mousePressEvent(QMouseEvent *event) {
    m_mouse_pressed = true;
    m_cursor_press_pos = get_grid_pos(event->position().toPoint());
}

void RoutingGridWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (m_mouse_pressed && m_cursor_hovering_grid) {
        // Check if a route already exists for sink
        // Emit try_create_route if new route, Emit try_move_route if route alredy exists

        if (route_exists_on_row(m_cursor_grid_pos.y())) {
            move_cell_on_row(m_cursor_press_pos.y(), m_cursor_press_pos.x());
            emit try_move_route({}, {}, {});
        } else {
            add_pending_cell(m_cursor_press_pos);
            emit try_create_route({}, {});
        }

        update();
    }

    m_mouse_pressed = false;
}

RoutingGridMetrics RoutingGridWidget::compute_metrics_for_zone(const QRect &zone) {
    RoutingGridMetrics metrics{0, 0};
    metrics.zone_max_cols = (zone.width() - RoutingWidgetConfig::heading_length) / RoutingWidgetConfig::cell_width_height;
    metrics.zone_max_cols = std::min(metrics.zone_max_cols + 1, static_cast<int>(m_columns.size()));

    metrics.zone_max_rows = (zone.height() - RoutingWidgetConfig::heading_length) / RoutingWidgetConfig::cell_width_height;
    metrics.zone_max_rows = std::min(metrics.zone_max_rows + 1, static_cast<int>(m_rows.size()));

    metrics.x_max = RoutingWidgetConfig::heading_length + (metrics.zone_max_cols) * RoutingWidgetConfig::cell_width_height;
    metrics.y_max = RoutingWidgetConfig::heading_length + (metrics.zone_max_rows) * RoutingWidgetConfig::cell_width_height;

    return metrics;
}

void RoutingGridWidget::draw_grid(QPainter *painter) const {
    painter->save();

    QPen pen = painter->pen();
    pen.setColor(ThemeColors::grid_color_strong);
    pen.setCosmetic(true);
    pen.setWidth(1);
    painter->setPen(pen);

    for (int v = 0; v < m_grid_metrics.zone_max_cols + 1; v++) {
        int x_pos = RoutingWidgetConfig::heading_length + v * RoutingWidgetConfig::cell_width_height;
        painter->drawLine(x_pos, 0, x_pos, m_grid_metrics.y_max);
    }

    for (int h = 0; h < m_grid_metrics.zone_max_rows + 1; h++) {
        int y_pos = RoutingWidgetConfig::heading_length + h * RoutingWidgetConfig::cell_width_height;
        painter->drawLine(0, y_pos, m_grid_metrics.x_max, y_pos);
    }

    painter->restore();
}

void RoutingGridWidget::draw_text(QPainter *painter) const {
    painter->save();

    for (int v = 0; v < m_grid_metrics.zone_max_cols; v++) {
        int x_pos = RoutingWidgetConfig::heading_length + (v + 1) * RoutingWidgetConfig::cell_width_height;
        QRect text_rect{
            0, 0,
            RoutingWidgetConfig::heading_length - RoutingWidgetConfig::heading_margin, RoutingWidgetConfig::cell_width_height
        };

        painter->save();
        painter->translate(x_pos, 0);
        painter->rotate(90);
        painter->drawText(text_rect, Qt::AlignRight | Qt::AlignVCenter, m_columns[v].name);
        painter->restore();
    }

    painter->restore();

    for (int h = 0; h < m_grid_metrics.zone_max_rows; h++) {
        int y_pos = RoutingWidgetConfig::heading_length + h * RoutingWidgetConfig::cell_width_height;
        QRect text_rect{
            0, y_pos,
            RoutingWidgetConfig::heading_length - RoutingWidgetConfig::heading_margin, RoutingWidgetConfig::cell_width_height
        };

        painter->drawText(text_rect, Qt::AlignRight | Qt::AlignVCenter, m_rows[h].name);
    }
}


void RoutingGridWidget::draw_dots(QPainter *painter) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const QColor pending_color = QColor{Qt::yellow};
    const QColor valid_color = QColor{Qt::green};

    QRect dot_rect = QRect{
        3,
        3,
        RoutingWidgetConfig::cell_width_height - 6,
        RoutingWidgetConfig::cell_width_height - 6
    };

    for (auto& cell : m_grid_state) {
        int dot_x = RoutingWidgetConfig::heading_length + cell.grid_pos.x() * RoutingWidgetConfig::cell_width_height;
        int dot_y = RoutingWidgetConfig::heading_length + cell.grid_pos.y() * RoutingWidgetConfig::cell_width_height;

        QColor dot_color = cell.state == RoutingCellState::ROUTE_VALID ? valid_color : pending_color;

        QPen pen = painter->pen();
        pen.setColor(dot_color);

        painter->setPen(pen);
        painter->setBrush(dot_color);

        painter->drawEllipse(dot_rect.translated(dot_x, dot_y));
    }

    if (m_cursor_hovering_grid) {
        int dot_x = RoutingWidgetConfig::heading_length + m_cursor_grid_pos.x() * RoutingWidgetConfig::cell_width_height;
        int dot_y = RoutingWidgetConfig::heading_length + m_cursor_grid_pos.y() * RoutingWidgetConfig::cell_width_height;

        QPen pen = painter->pen();
        pen.setColor(ThemeColors::handle_selected_color);

        painter->setPen(pen);
        painter->setBrush(QBrush{QColor{ThemeColors::handle_selected_color}});
        painter->drawEllipse(dot_rect.translated(dot_x, dot_y));
    }

    painter->restore();
}

void RoutingGridWidget::add_pending_cell(QPoint grid_pos) {
    m_grid_state.push_back(RoutingCell{{}, {}, grid_pos, RoutingCellState::ROUTE_PENDING});
}

void RoutingGridWidget::validate_route(const RouteNode& from, const RouteNode& to) {
    for (auto& cell : m_grid_state) {
        if (cell.from == from && cell.to == to) {
            cell.state = RoutingCellState::ROUTE_VALID;
            update();

            return;
        }
    }
}

void RoutingGridWidget::add_node(QString node_name, RoutingMode mode, const RouteNode& routing_node) {
    RoutingNodeMeta nmeta = {
        std::move(node_name),
        routing_node
    };

    if (mode == RoutingMode::ROUTING_SOURCE) {
        m_columns.push_back(nmeta);
    } else if (mode == RoutingMode::ROUTING_SINK) {
        m_rows.push_back(nmeta);
    }
}

bool RoutingGridWidget::route_exists_on_row(const int row) {
    return std::find_if(m_grid_state.begin(), m_grid_state.end(), [row](const RoutingCell& pred) {
        return pred.grid_pos.y() == row;
    }) != m_grid_state.end();
}

void RoutingGridWidget::move_cell_on_row(int row, int new_col) {
    auto existing_route = std::find_if(m_grid_state.begin(), m_grid_state.end(), [row](const RoutingCell& pred) {
        return pred.grid_pos.y() == row;
    });

    existing_route->grid_pos.setX(new_col);
    existing_route->state = RoutingCellState::ROUTE_PENDING;
}
