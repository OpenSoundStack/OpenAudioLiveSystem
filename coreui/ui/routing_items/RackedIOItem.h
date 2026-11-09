// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef RACKEDIO_ITEM_H
#define RACKEDIO_ITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QFont>

#include <vector>
#include <cassert>

namespace DrawingParameters {
    constexpr int col_width = 200;
    constexpr int row_height = 30;

    constexpr int elem_height = row_height * 0.80f;
    constexpr int elem_offset = static_cast<int>((row_height * 0.20f) / 2.0f);
    constexpr float x_width = (-static_cast<float>(2 * elem_offset) / static_cast<float>(col_width)) + 1.0f;
    constexpr int elem_width = static_cast<int>(x_width * col_width);
}

enum class BoardType {
    BOARD_EMPTY,
    BOARD_IO,
    BOARD_DSP
};

class RackedIOItem : public QGraphicsItem {
public:
    RackedIOItem(int cols, int rows);
    ~RackedIOItem() override = default;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void populate_slot(BoardType btype, uint8_t flag_map, int col, int row);
private:
    static void draw_xlr(QPainter* painter, QPoint pos, int width, bool is_input);
    static void draw_io_board(QPainter* painter, QPoint pos, uint8_t inout_map);
    static void draw_empty_slot(QPainter* painter, QPoint pos);
    static void draw_dsp(QPainter* painter, QPoint pos);

    void init_slots();

    int m_cols;
    int m_rows;

    std::vector<std::vector<std::pair<BoardType, uint8_t>>> m_boards;
};


#endif // RACKEDIO_ITEM_H
