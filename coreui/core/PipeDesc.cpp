#include "PipeDesc.h"

PipeElemDesc::PipeElemDesc(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(80);
    setMaximumHeight(200);
}

void PipeElemDesc::paintEvent(QPaintEvent *event) {
    auto* painter = new QPainter{this};

    QRect zone = event->rect();
    zone.translate(QPoint{0, 2});
    zone.setHeight(zone.height() - 2);

    render_elem(zone, painter);
    painter->end();

    QWidget::paintEvent(event);
}

