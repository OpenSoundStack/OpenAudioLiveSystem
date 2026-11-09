#include "RackedIOItem.h"

RackedIOItem::RackedIOItem(int cols, int rows) : QGraphicsItem() {
    m_cols = cols;
    m_rows = rows;

    init_slots();
}

QRectF RackedIOItem::boundingRect() const {
    return {};
}

void RackedIOItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    QRect rack_zone = {0, 0, DrawingParameters::col_width * m_cols, DrawingParameters::row_height * m_rows};
    QRect elem_rect = {
        DrawingParameters::elem_offset, DrawingParameters::elem_offset,
        DrawingParameters::elem_width, DrawingParameters::elem_height
    };

    QPen black_pen = painter->pen();
    black_pen.setColor(Qt::black);

    painter->setPen(black_pen);
    painter->setBrush(QBrush{0x404040});

    painter->drawRect(rack_zone);

    for (int y = 0; y < m_rows; y++) {
        for (int x = 0; x < m_cols; x++) {
            QPoint board_pos = {
                x * DrawingParameters::col_width + DrawingParameters::elem_offset,
                y * DrawingParameters::row_height + DrawingParameters::elem_offset
            };

            auto board = m_boards[x][y];
            switch (board.first) {
                case BoardType::BOARD_DSP:
                    draw_dsp(painter, board_pos);
                    break;
                case BoardType::BOARD_IO:
                    draw_io_board(painter, board_pos, board.second);
                    break;
                case BoardType::BOARD_EMPTY:
                    draw_empty_slot(painter, board_pos);
                    break;
            }
        }
    }
}

void RackedIOItem::draw_io_board(QPainter *painter, QPoint pos, uint8_t inout_map) {
    QTransform origin = painter->transform();
    QTransform local_origin = origin;
    local_origin.translate(pos.x(), pos.y());

    painter->setTransform(local_origin);

    auto tr_rect = QRect(0, 0, DrawingParameters::elem_width, DrawingParameters::elem_height);
    painter->setBrush(QBrush{0x262626});
    painter->drawRect(tr_rect);

    const int conn_width = tr_rect.width() / 8;
    const int draw_width = (int)(static_cast<float>(conn_width) * 0.80f);
    const int offset_x = (int)(static_cast<float>(conn_width) * 0.05f);

    for (int i = 0; i < 8; i++) {
        draw_xlr(
            painter,
    {offset_x + tr_rect.x() + conn_width * i + conn_width / 2, tr_rect.y() + conn_width / 2},
            draw_width,
            inout_map & (1 << i)
        );
    }

    painter->setTransform(origin);
}

void RackedIOItem::draw_empty_slot(QPainter *painter, QPoint pos) {
    QTransform origin = painter->transform();
    QTransform local_origin = origin;
    local_origin.translate(pos.x(), pos.y());

    painter->setTransform(local_origin);

    auto tr_rect = QRect(0, 0, DrawingParameters::elem_width, DrawingParameters::elem_height);
    painter->setBrush(QBrush{0x101010});
    painter->drawRect(tr_rect);

    painter->setTransform(origin);
}

void RackedIOItem::draw_xlr(QPainter *painter, QPoint pos, int width, bool is_input) {
    QTransform default_tr = painter->transform();

    QTransform drawing_tr = default_tr;
    drawing_tr.translate(pos.x(), pos.y());
    drawing_tr.scale(static_cast<float>(width) / 500.0f, static_cast<float>(width) / 500.0f);
    painter->setTransform(drawing_tr);

    QRect conn_rect{-250, -250, 500, 500};
    QRect pin_rect{-75/2, -75/2, 75, 75};

    QColor back_color = is_input ? QColor{0x1F1F1F} : QColor{0x121212};
    QColor pin_color = is_input ? QColor{Qt::black} : QColor{0x8f711f};
    QColor pen_color = is_input ? QColor{Qt::black} : QColor{0x3B3B3B};

    QPen pen = painter->pen();
    pen.setColor(pen_color);
    pen.setWidth(40);

    painter->setPen(pen);
    painter->setBrush(back_color);

    painter->drawEllipse(conn_rect);

    pen.setWidth(1);
    pen.setColor(Qt::black);
    painter->setPen(pen);

    painter->setBrush(pin_color);
    painter->drawEllipse(pin_rect.translated(-125, 0));
    painter->drawEllipse(pin_rect.translated(125, 0));
    painter->drawEllipse(pin_rect.translated(0, 125));

    painter->setTransform(default_tr);
}

void RackedIOItem::draw_dsp(QPainter *painter, QPoint pos) {
    QTransform origin = painter->transform();
    QTransform local_origin = origin;
    local_origin.translate(pos.x(), pos.y());

    QPen pen = painter->pen();
    QPen white_pen = pen;
    white_pen.setColor(Qt::white);

    painter->setTransform(local_origin);

    auto tr_rect = QRect(0, 0, DrawingParameters::elem_width, DrawingParameters::elem_height);
    painter->setBrush(QBrush{0x262626});
    painter->drawRect(tr_rect);

    auto led_rect = QRect(0.10f * DrawingParameters::elem_width - 3, DrawingParameters::elem_height / 2 - 3, 6, 6);

    painter->setBrush(Qt::darkGreen);
    painter->drawEllipse(led_rect);

    painter->setBrush(Qt::darkRed);
    painter->drawEllipse(led_rect.translated(20, 0));

    QFont painter_font = painter->font();
    QFont local_font = painter_font;
    local_font.setItalic(true);
    local_font.setBold(true);
    local_font.setPointSizeF(12);

    QFontMetrics metrics{local_font};
    QRect brect = metrics.boundingRect("DSP");
    float text_offset = brect.height() / 2.0f - 3;

    QPointF text_pos = QPointF{
        DrawingParameters::elem_width * 0.55f,
        DrawingParameters::elem_height - text_offset
    };

    painter->setPen(white_pen);
    painter->setFont(local_font);
    painter->drawText(brect.translated(text_pos.x(), text_pos.y()), Qt::AlignVCenter, "DSP");
    painter->setFont(painter_font);
    painter->setPen(pen);

    painter->setTransform(origin);
}

void RackedIOItem::init_slots() {
    for (int x = 0; x < m_cols; x++) {
        std::vector<std::pair<BoardType, uint8_t>> col;
        for (int y = 0; y < m_rows; y++) {
            col.emplace_back(BoardType::BOARD_EMPTY, 0x00);
        }

        m_boards.emplace_back(col);
    }
}

void RackedIOItem::populate_slot(BoardType btype, uint8_t flag_map, int col, int row) {
    assert(col < m_cols && "Out of bounds access");
    assert(row < m_rows && "Out of bounds access");

    m_boards[col][row].first = btype;
    m_boards[col][row].second = flag_map;
}
