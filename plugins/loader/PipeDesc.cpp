// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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

// -------------------------------------------------------------------------------

PipeElemDesc::PipeElemDesc(AudioRouter* router, QString plug_name, QWidget *parent) : QWidget(parent) {
    setMinimumHeight(80);
    setMaximumHeight(200);

    m_being_clicked = false;
    m_selected = false;
    m_controls = nullptr;

    m_router = router;

    m_channel = 0;
    m_index = 0;
    m_dsp_host = 0;

    m_pname = std::move(plug_name);

    m_flags = ElemFlags::ELEM_NO_FLAGS;
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

    if(!m_selected) {
        pen.setColor(Qt::white);
    } else {
        pen.setColor(Qt::yellow);
    }

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

ElemFlags PipeElemDesc::get_flags() {
    return m_flags;
}

void PipeElemDesc::receive_feedback_control(const ControlPacket &pck) {

}

QJsonObject PipeElemDesc::get_plugin_state_json() {
    QJsonObject jobj{};
    jobj["plugname"] = m_pname;

    QJsonArray control_data_array;
    for (auto& c : m_control_data) {
        QJsonObject control_data_obj;
        control_data_obj["index"] = c.first;

        auto bin_data = c.second->get_data_binary();
        QByteArray raw_data(
            reinterpret_cast<char *>(bin_data.data()),
            bin_data.size()
        );

        QString b64_data = raw_data.toBase64();
        control_data_obj["binval"] = b64_data;

        control_data_array.append(control_data_obj);
    }

    jobj["controls"] = control_data_array;

    return jobj;
}

bool PipeElemDesc::load_plugin_state_json(const QJsonObject &jobj) {
    if (!jobj.contains("plugname") || jobj["plugname"] != m_pname) {
        return false;
    }

    QJsonArray arr = jobj["controls"].toArray();
    for (const auto e : arr) {
        QJsonObject arr_control_obj = e.toObject();
        int control_idx = arr_control_obj["index"].toInt();

        if (m_control_data.contains(control_idx)) {
            QString b64_binval = arr_control_obj["binval"].toString();
            QByteArray bin_data_array = QByteArray::fromBase64(b64_binval.toUtf8());

            std::span<uint8_t> bin_data(
                reinterpret_cast<uint8_t*>(bin_data_array.data()),
                bin_data_array.size()
            );

            if (!m_control_data[control_idx]->load_data_binary(bin_data)) {
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}
