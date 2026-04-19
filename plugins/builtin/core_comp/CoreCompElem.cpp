// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompElem.h"

#include <QPainterPath>

CoreCompElem::CoreCompElem(AudioRouter *router) : PipeElemDesc(router) {
    m_comp_ui = new CoreComp_UI();
    m_controls = m_comp_ui;

    m_base_params = CompDefaultParams::static_defaults;
    m_time_params = CompDefaultParams::dyn_defaults;

    connect(m_comp_ui, &CoreComp_UI::comp_changed, this, [this](const CompStaticParams& params) {
        m_base_params = params;
        m_static_params->set_data(m_base_params);

        update();
        send_control_packets();
    });

    connect(m_comp_ui, &CoreComp_UI::comp_time_changed, this, [this](const CompDynamicsParams& params) {
        m_time_params = params;
        m_dyn_params->set_data(m_time_params);

        send_control_packets();
    });

    setup_dsp_link();
}

void CoreCompElem::render_elem(QRect zone, QPainter *painter) {
    QRect transfer_zone = zone;
    transfer_zone.setWidth(zone.height() * 0.90f);
    transfer_zone.setHeight(zone.height() * 0.90f);

    transfer_zone.translate(
        zone.width() * 0.05f,
        (zone.height() - transfer_zone.height()) / 2
    );

    draw_background(painter, zone);

    CompViz::draw_comp_curve(painter, transfer_zone, m_base_params.threshold, m_base_params.ratio, m_base_params.gain);

    draw_frame(painter, transfer_zone);
    draw_frame(painter, zone);
}

void CoreCompElem::setup_dsp_link() {
    m_static_params = std::make_shared<GenericElemControlData<CompStaticParams>>(m_base_params);
    m_dyn_params = std::make_shared<GenericElemControlData<CompDynamicsParams>>(m_time_params);

    register_control(1, m_static_params);
    register_control(2, m_dyn_params);
}

void CoreCompElem::receive_feedback_control(const ControlPacket &pck) {
    float gain = 0.0f;
    float level_db = 0.0f;

    memcpy(&gain, &pck.packet_data.data[0], sizeof(float));
    memcpy(&level_db, &pck.packet_data.data[1], sizeof(float));

    float gain_db = 10.0f * std::log10(gain);
}
