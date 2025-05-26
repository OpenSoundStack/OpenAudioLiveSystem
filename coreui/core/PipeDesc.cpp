#include "PipeDesc.h"

PipeDesc::~PipeDesc() {
    delete desc_content;
    if (next_pipe_elem.has_value()) {
        delete next_pipe_elem.value();
    }
}

void PipeDesc::index_pipes() {
    PipeDesc* desc = this;

    int index = 0;
    while (desc != nullptr) {
        desc->desc_content->index_pipe(index);

        if (desc->next_pipe_elem.has_value()) {
            desc = desc->next_pipe_elem.value();
            index++;
        } else {
            desc = nullptr;
        }
    }
}

void PipeDesc::set_pipe_channel(uint8_t channel) {
    PipeDesc* desc = this;

    int index = 0;
    while (desc != nullptr) {
        desc->desc_content->set_channel(channel);

        if (desc->next_pipe_elem.has_value()) {
            desc = desc->next_pipe_elem.value();
            index++;
        } else {
            desc = nullptr;
        }
    }
}


PipeElemDesc::PipeElemDesc(QWidget *parent) : QWidget(parent) {
    setMinimumHeight(80);
    setMaximumHeight(200);

    m_being_clicked = false;
    m_selected = false;
    m_controls = nullptr;

    m_channel = 0;
    m_index = 0;
}

void PipeElemDesc::paintEvent(QPaintEvent *event) {
    auto* painter = new QPainter{this};

    QRect zone = event->rect();
    zone.translate(QPoint{0, 2});
    zone.setHeight(zone.height() - 2);

    render_elem(zone, painter);
    painter->end();

    delete painter;

    QWidget::paintEvent(event);
}

void PipeElemDesc::mousePressEvent(QMouseEvent *event) {
    m_being_clicked = true;
}

void PipeElemDesc::mouseReleaseEvent(QMouseEvent *event) {
    m_being_clicked = false;

    if (m_controls != nullptr && rect().contains(event->pos())) {
        emit elem_selected();
    }
}

QWidget *PipeElemDesc::get_controllable_widget() {
    return m_controls;
}

void PipeElemDesc::draw_background(QPainter *painter, QRect zone) {
    painter->setBrush(QBrush{QColor{0x2E2E2E}});
    painter->drawRect(zone);
}

void PipeElemDesc::draw_frame(QPainter *painter, QRect zone) {
    QPen pen = painter->pen();
    pen.setColor(Qt::white);
    pen.setWidth(1);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(zone);
}

void PipeElemDesc::index_pipe(int index) {
    m_index = index;
}

int PipeElemDesc::get_index() {
    return m_index;
}

void PipeElemDesc::set_channel(uint8_t channel) {
    m_channel = channel;
}

uint8_t PipeElemDesc::get_channel() {
    return m_channel;
}
