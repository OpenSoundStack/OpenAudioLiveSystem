//  This file is part of the Open Audio Live System project, a live audio environment
//  Copyright (c) 2026 - Mathis DELGADO
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, version 3 of the License.
//
//  This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.

#include "DebuggerWindow.h"
#include "ui_DebuggerWindow.h"


DebuggerWindow::DebuggerWindow(QWidget *parent) : QWidget(parent), ui(new Ui::DebuggerWindow) {
    ui->setupUi(this);

    m_rendered_packets_count = 0;
    m_is_recording = false;

    init_scope_scene();
    init_stats();
    init_network();
}

DebuggerWindow::~DebuggerWindow() {
    delete ui;
}

void DebuggerWindow::reset_min_max() {
    m_packet_min_delta = 0xFFFFFFFFFFFFFFFF;
    m_packet_max_delta = 0;
}

void DebuggerWindow::init_stats() {
    reset_min_max();
    m_last_stamp = 0;
    m_first_pck_received = false;
}

void DebuggerWindow::init_network() {
    const char dev_name[32] = "DEBUGGER";

    m_pconf.iface = "enp34s0";
    m_pconf.sample_rate = SamplingRate::SAMPLING_96K;
    m_pconf.dev_type = DeviceType::MONITORING;
    m_pconf.uid = ui->self_id->value();
    m_pconf.topo.phy_in_count = 1;
    m_pconf.topo.phy_out_count = 1;
    m_pconf.topo.pipes_count = 1;
    m_pconf.ck_type = CKTYPE_SLAVE;
    memcpy(&m_pconf.dev_name, dev_name, 32);

    m_nmapper = std::make_shared<NetworkMapper>(m_pconf);

    std::cout << "Initializing on " << m_pconf.iface << std::endl;
    if (m_nmapper->init_mapper(m_pconf.iface)) {
        m_nmapper->launch_mapping_process();
    } else {
        QMessageBox::critical(this, "ERROR", "Failed to init network mapper");
        quick_exit(-1);
    }

    m_audio_socket = std::make_shared<LowLatSocket>(m_pconf.uid, m_nmapper);
    m_audio_socket->init_socket(m_pconf.iface, EthProtocol::ETH_PROTO_OANAUDIO);

    std::thread audio_thread = std::thread([this]() {
        LowLatPacket<AudioPacket> rx_packet;
        int counter = 0;

        while (true) {
            if (m_audio_socket->receive_data(&rx_packet) > 0) {
                update_stats();
                emit stats_changed();

                if (m_is_recording) {
                    emit audio_received(rx_packet.payload.packet_data);
                }
            }
        }
    });
    audio_thread.detach();

    connect(this, &DebuggerWindow::stats_changed, this, [this]() {
        ui->jitter->setValue((m_packet_max_delta - m_packet_min_delta) / 1000.0f);
        ui->max_delta->setValue(m_packet_max_delta / 1000.0f);
        ui->min_delta->setValue(m_packet_min_delta / 1000.0f);
        ui->mean_delta->setValue(m_mean_delta / 1000.0f);
    });

    connect(this, &DebuggerWindow::audio_received, this, [this](AudioData data) {
        scope_render_audio(data);
    });

    connect(ui->min_max_rst, &QPushButton::clicked, this, [this]() {
        reset_min_max();
    });

    connect(ui->rec_start, &QPushButton::clicked, this, [this]() {
        m_is_recording = true;
        m_stream_scope_scene->clear();

        ui->rec_start->setEnabled(false);
        ui->rec_stop->setEnabled(true);
    });

    connect(ui->rec_stop, &QPushButton::clicked, this, [this]() {
        m_is_recording = false;

        ui->rec_start->setEnabled(true);
        ui->rec_stop->setEnabled(false);
    });
}

void DebuggerWindow::update_stats() {
    auto now = m_nmapper->local_now_us();

    if (m_first_pck_received) {
        auto delta = now - m_last_stamp;
        m_packet_deltas.push_back(delta);

        if (delta > m_packet_max_delta) {
            m_packet_max_delta = delta;
        }

        if (delta < m_packet_min_delta) {
            m_packet_min_delta = delta;
        }

        // Mean measurement over 500 packets
        // Reset min max measure too
        if (m_packet_deltas.size() > 500) {
            m_packet_deltas.pop_front();
        }

        float delta_sum = 0;
        for (auto& d : m_packet_deltas) {
            delta_sum += d;
        }
        delta_sum /= m_packet_deltas.size();
        m_mean_delta = delta_sum;
    }

    m_last_stamp = now;
    m_first_pck_received = true;
}

void DebuggerWindow::init_scope_scene() {
    m_stream_scope_scene = new QGraphicsScene{};
    ui->stream_view->setScene(m_stream_scope_scene);
}

void DebuggerWindow::scope_render_audio(const AudioData& data) {
    const Qt::GlobalColor colors[3] = {Qt::red, Qt::cyan, Qt::green};
    static int color_counter = 0;

    QGraphicsScene* scene = ui->stream_view->scene();

    QPen pen{colors[color_counter]};
    pen.setWidth(1);

    float x_start = m_rendered_packets_count * 5.0f * AUDIO_DATA_SAMPLES_PER_PACKETS;
    float x_end = (m_rendered_packets_count + 1) * 5.0f * AUDIO_DATA_SAMPLES_PER_PACKETS;
    scene->addLine(x_start, 0, x_end, 0, pen);

    for (int i = 0; i < AUDIO_DATA_SAMPLES_PER_PACKETS; i++) {
        float sample_y = data.samples[i] * 500.0f;
        float sample_x = x_start + i * 5.0f;

        scene->addLine(sample_x, 0, sample_x, sample_y, pen);
    }

    m_rendered_packets_count++;
    color_counter = (color_counter + 1) % 3;
}
