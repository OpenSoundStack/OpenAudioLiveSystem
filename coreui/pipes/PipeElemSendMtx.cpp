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

    FaderSendMtx* m_fader_send_mtx = new FaderSendMtx();
    m_controls = m_fader_send_mtx;

    m_sm = sm;

    m_flags = ElemFlags::ELEM_IS_OUTPUT_MATRIX;

    find_buses();
}

void PipeElemSendMtx::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);

    painter->drawText(zone, Qt::AlignCenter, "Send Matrix");

    draw_frame(painter, zone);
}

void PipeElemSendMtx::find_buses() {
    auto show = m_sm->get_show();

    int bus_count = 0;
    for(auto& pipe : show) {
        auto first_elem = pipe->get_pipe_desc()->desc_content;

        if(first_elem && (first_elem->get_flags() & ELEM_IS_INPUT_MATRIX)) {
            bus_count++;
        }
    }

    std::cout << "FOUND " << bus_count << " BUS" << std::endl;
}