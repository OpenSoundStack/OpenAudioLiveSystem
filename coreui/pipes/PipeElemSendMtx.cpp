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

#include "PipeElemSendMtx.h"
#include "coreui/core/ShowManager.h"

PipeElemSendMtx::PipeElemSendMtx(ShowManager* sm, AudioRouter *router) : PipeElemDesc(router) {
    setFixedHeight(20);

    m_fader_mtx = new FaderSendMtx();
    m_controls = m_fader_mtx;

    connect(m_fader_mtx, &FaderSendMtx::fader_value_changed, [this](uint8_t channel, uint16_t host, float value) {
        FaderControlFrame cf{};
        cf.channel = channel;
        cf.host = host;
        cf.level = value;

        auto control_pck = construct_fader_packet(cf);
        m_router->send_control_packet(control_pck, get_host());
    });

    m_flags = ElemFlags::ELEM_IS_OUTPUT_MATRIX;

    m_sm = sm;
    connect(m_sm, &ShowManager::pipe_added, this, [this](PipeVisualizer* desc) {
        if(is_bus(desc->get_pipe_desc())) {
            m_buses.append(desc);
            m_fader_mtx->add_fader(desc->get_name(), desc->get_channel(), desc->get_host());
        }
    });

    find_buses();
}

void PipeElemSendMtx::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);

    painter->drawText(zone, Qt::AlignCenter, "Send Matrix");

    draw_frame(painter, zone);
}

void PipeElemSendMtx::find_buses() {
    auto show = m_sm->get_show();

    for(auto& pipe : show) {
        if(is_bus(pipe->get_pipe_desc())) {
            m_fader_mtx->add_fader(pipe->get_name(), pipe->get_channel(), pipe->get_host());
        }
    }
}

bool PipeElemSendMtx::is_bus(PipeDesc *desc) {
    auto* first_elem = desc->desc_content;

    // If it has an input matrix (input that can get multiple inputs and sum them)
    // on pipe first element, it is considered as a bus
    return first_elem && (first_elem->get_flags() & ElemFlags::ELEM_IS_INPUT_MATRIX);
}

ControlPacket PipeElemSendMtx::construct_fader_packet(FaderControlFrame& control_frame) {
    ControlPacket pck{};
    pck.header.type = PacketType::CONTROL;
    pck.packet_data.channel = get_channel();
    pck.packet_data.elem_index = get_index();
    pck.packet_data.control_id = 0;
    pck.packet_data.control_type = DataTypes::CUSTOM;

    memcpy(pck.packet_data.data, &control_frame, sizeof(FaderControlFrame));

    return std::move(pck);
}
