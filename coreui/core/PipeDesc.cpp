#include "PipeDesc.h"

PipeElemDesc::PipeElemDesc(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(40);
    setMaximumHeight(150);
}

void PipeElemDesc::paintEvent(QPaintEvent *event) {
    auto* painter = new QPainter{this};
    render_elem(event->rect(), painter);
    painter->end();

    QWidget::paintEvent(event);
}

