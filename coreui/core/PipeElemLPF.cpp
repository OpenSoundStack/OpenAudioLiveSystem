#include "PipeElemLPF.h"

#include "PipeDesc.h"

PipeElemLPF::PipeElemLPF(float cutoff) : PipeElemDesc(nullptr) {
    m_cutoff = cutoff;
}

void PipeElemLPF::set_cutoff(float cutoff) {
    m_cutoff = cutoff;
}

void PipeElemLPF::render_elem(QRect zone, QPainter *painter) {
    constexpr int stroke_color = 0xB467F0;
    constexpr int fill_color = 0xE3CBF5;

    painter->setBrush(QBrush{QColor{0x2E2E2E}});
    painter->drawRect(zone);

    float x_pos = freq_to_log_scale(m_cutoff) * zone.width();

    QPoint start_point = QPoint{1, zone.height() / 2};
    QPoint cutoff_point = QPoint{(int)x_pos, zone.height() / 2};
    QPoint min_inf_db_point = QPoint{(int)(cutoff_point.x() + 0.2f * zone.width()), zone.height() + 3};
    QPoint end_point = QPoint{1, zone.height() + 3};

    QPainterPath path = QPainterPath{};
    path.moveTo(start_point);
    path.lineTo(cutoff_point);
    path.lineTo(min_inf_db_point);

    QPen pen = painter->pen();
    pen.setColor(QColor{stroke_color});
    pen.setWidth(2);

    painter->setPen(pen);
    painter->drawPath(path);

    path.lineTo(end_point);
    painter->fillPath(path, QColor{fill_color});

    // Frame
    pen.setColor(Qt::white);
    pen.setWidth(1);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(zone);
}
