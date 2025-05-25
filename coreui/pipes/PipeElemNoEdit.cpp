#include "PipeElemNoEdit.h"

PipeElemNoEdit::PipeElemNoEdit(QString block_name) {
    m_controls = nullptr;
    m_block_name = block_name;

    setFixedHeight(20);
}

void PipeElemNoEdit::render_elem(QRect zone, QPainter *painter) {
    painter->setBrush(QBrush{QColor{0x2E2E2E}});
    painter->drawRect(zone);

    painter->drawText(zone, Qt::AlignCenter, m_block_name);

    // Frame
    QPen pen = painter->pen();
    pen.setColor(Qt::white);
    pen.setWidth(1);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(zone);
}
