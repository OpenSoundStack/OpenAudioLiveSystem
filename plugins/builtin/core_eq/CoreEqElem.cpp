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

#include "CoreEqElem.h"

CoreEqElem::CoreEqElem(AudioRouter *router) : PipeElemDesc(router) {
    auto* control_ui = new CoreEqControlUI{};
    m_controls = control_ui;

    m_peak_control = std::make_shared<GenericElemControlData<PeakFilterData>>(
        PeakFilterData{1000.0f, 0}
    );
    register_control(1, m_peak_control);

    connect(control_ui, &FilterEditBase::handle_moved, this, [this](float fc, float gain) {
        PeakFilterData data{fc, gain};
        m_peak_control->set_data(data);

        send_control_packets();
    });
}

void CoreEqElem::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);



    draw_frame(painter, zone);
}
