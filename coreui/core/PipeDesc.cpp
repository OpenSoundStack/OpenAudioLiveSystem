// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

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
        if (desc->desc_content != nullptr) {
            desc->desc_content->index_pipe(index);
        }

        if (desc->next_pipe_elem.has_value()) {
            desc = desc->next_pipe_elem.value();
            index++;
        } else {
            desc = nullptr;
        }
    }
}

void PipeDesc::set_pipe_channel(uint8_t channel, uint16_t host) {
    PipeDesc* desc = this;

    int index = 0;
    while (desc != nullptr) {
        desc->desc_content->set_channel(channel);
        desc->desc_content->set_host(host);

        if (desc->next_pipe_elem.has_value()) {
            desc = desc->next_pipe_elem.value();
            index++;
        } else {
            desc = nullptr;
        }
    }
}


PipeElemDesc::PipeElemDesc(AudioRouter* router, QWidget *parent) : QWidget(parent) {
    setMinimumHeight(80);
    setMaximumHeight(200);

    m_being_clicked = false;
    m_selected = false;
    m_controls = nullptr;

    m_router = router;

    m_channel = 0;
    m_index = 0;
    m_dsp_host = 0;
}

void PipeElemDesc::paintEvent(QPaintEvent *event) {
    auto* painter = new QPainter{this};
    painter->setRenderHint(QPainter::Antialiasing);

    QRect zone = event->rect();
    zone.translate(QPoint{2, 2});
    zone.setHeight(zone.height() - 4);
    zone.setWidth(zone.width() - 4);

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

void PipeElemDesc::set_host(uint16_t host) {
    m_dsp_host = host;
}

uint16_t PipeElemDesc::get_host() {
    return m_dsp_host;
}

void PipeElemDesc::send_control_packets() {
    ControlPacket pck{};
    pck.header.type = PacketType::CONTROL;
    pck.packet_data.channel = get_channel();
    pck.packet_data.control_id = 0;
    pck.packet_data.elem_index = get_index();

    for (auto& ctrl : m_control_data) {
        if (!ctrl.second->has_changed()) {
            continue;
        }

        pck.packet_data.control_id = ctrl.first;
        ctrl.second->fill_packet(pck);
        ctrl.second->reset_changed_flag();

        m_router->send_control_packet(pck, get_host());
    }
}

void PipeElemDesc::register_control(uint8_t control_id, std::shared_ptr<ElemControlData> control_data) {
    m_control_data[control_id] = std::move(control_data);
}
