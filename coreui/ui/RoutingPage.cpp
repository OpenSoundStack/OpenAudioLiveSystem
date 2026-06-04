// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "RoutingPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QString>
#include <QScrollArea>

#include "OpenAudioNetwork/common/packet_structs.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"

#include "../core/DSPManager.h"

namespace {
const char* device_type_tag(DeviceType t) {
    switch (t) {
        case DeviceType::CONTROL_SURFACE:    return "UI";
        case DeviceType::MONITORING:         return "MON";
        case DeviceType::AUDIO_IO_INTERFACE: return "IO";
        case DeviceType::AUDIO_DSP:          return "DSP";
    }
    return "?";
}

QString peer_label(const PeerInfos& p) {
    return QStringLiteral("[%1] %2 (0x%3)")
        .arg(device_type_tag(p.peer_data.type))
        .arg(QString::fromLocal8Bit(p.peer_data.dev_name, qstrnlen(p.peer_data.dev_name, 32)))
        .arg(p.peer_data.self_uid, 4, 16, QChar('0'));
}
} // namespace

RoutingPage::RoutingPage(ShowManager* sm, QWidget* parent)
    : QWidget(parent), m_sm(sm) {

    auto* root = new QVBoxLayout(this);

    auto* top_row = new QHBoxLayout();
    auto* back_btn = new QPushButton("Back");
    auto* refresh_btn = new QPushButton("Refresh peers");
    auto* clear_btn = new QPushButton("Clear all for source");
    top_row->addWidget(back_btn);
    top_row->addWidget(refresh_btn);
    top_row->addStretch();
    top_row->addWidget(clear_btn);
    root->addLayout(top_row);

    auto* selectors = new QHBoxLayout();
    selectors->addWidget(new QLabel("Source:"));
    m_source_box = new QComboBox();
    selectors->addWidget(m_source_box, 1);
    selectors->addWidget(new QLabel("Target:"));
    m_target_box = new QComboBox();
    selectors->addWidget(m_target_box, 1);
    root->addLayout(selectors);

    m_matrix_caption = new QLabel("Pick a source and a target to see the matrix.");
    root->addWidget(m_matrix_caption);

    // The matrix can get big (an engine peer advertises 64 in / 64 out),
    // so wrap it in a scroll area and anchor its grid to top-left so it
    // doesn't float to the centre when the page is bigger than the grid.
    m_matrix_host = new QWidget();
    m_matrix_layout = new QGridLayout(m_matrix_host);
    m_matrix_layout->setSpacing(2);
    m_matrix_layout->setContentsMargins(0, 0, 0, 0);
    m_matrix_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto* scroll = new QScrollArea();
    scroll->setWidget(m_matrix_host);
    scroll->setWidgetResizable(true);
    scroll->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    root->addWidget(scroll, 1);

    connect(back_btn, &QPushButton::clicked, this, &RoutingPage::back_requested);
    connect(refresh_btn, &QPushButton::clicked, this, &RoutingPage::refresh_peer_lists);
    connect(clear_btn, &QPushButton::clicked, this, &RoutingPage::clear_all_for_current_source);

    connect(m_source_box, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { rebuild_matrix(); });
    connect(m_target_box, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this](int) { rebuild_matrix(); });

    // Live-refresh dropdowns whenever a peer joins or leaves.
    connect(sm, &ShowManager::peer_change, this, [this](QString, int, bool) {
        refresh_peer_lists();
    });

    refresh_peer_lists();
}

void RoutingPage::refresh_peer_lists() {
    auto nmapper = m_sm->get_network_mapper();
    if (!nmapper) return;

    auto fill = [&](QComboBox* box) {
        const uint16_t prev = box->currentData().isValid()
            ? static_cast<uint16_t>(box->currentData().toUInt()) : 0;
        box->blockSignals(true);
        box->clear();
        for (const auto& p : nmapper->find_all_peers()) {
            box->addItem(peer_label(p), static_cast<uint>(p.peer_data.self_uid));
        }
        if (prev != 0) {
            int idx = box->findData(static_cast<uint>(prev));
            if (idx >= 0) box->setCurrentIndex(idx);
        }
        box->blockSignals(false);
    };
    fill(m_source_box);
    fill(m_target_box);

    rebuild_matrix();
}

void RoutingPage::rebuild_matrix() {
    // Wipe whatever was in the grid.
    while (auto* item = m_matrix_layout->takeAt(0)) {
        if (auto* w = item->widget()) w->deleteLater();
        delete item;
    }

    if (m_source_box->count() == 0 || m_target_box->count() == 0) {
        m_matrix_caption->setText("No discovered IO or DSP peers yet.");
        return;
    }

    const uint16_t src_uid = static_cast<uint16_t>(m_source_box->currentData().toUInt());
    const uint16_t dst_uid = static_cast<uint16_t>(m_target_box->currentData().toUInt());

    auto nmapper = m_sm->get_network_mapper();
    auto src_topo = nmapper->get_device_topo(src_uid);
    auto dst_topo = nmapper->get_device_topo(dst_uid);
    if (!src_topo || !dst_topo) {
        m_matrix_caption->setText("Selected peer has no advertised topology yet.");
        return;
    }

    const int rows = src_topo->phy_out_count;
    const int cols = dst_topo->phy_in_count;
    m_matrix_caption->setText(
        QStringLiteral("Routing %1 outputs (rows) → %2 inputs (cols).")
            .arg(rows).arg(cols));

    // Column headers (target inputs).
    for (int c = 0; c < cols; ++c) {
        auto* hdr = new QLabel(QString::number(c));
        hdr->setAlignment(Qt::AlignCenter);
        hdr->setFixedWidth(20);
        m_matrix_layout->addWidget(hdr, 0, c + 1);
    }

    // Look up which cells (if any) are active for this (src, dst) pair.
    const std::set<std::pair<uint8_t, uint8_t>>* active = nullptr;
    auto it = m_routes_by_source.find(src_uid);
    if (it != m_routes_by_source.end() && it->second.target_uid == dst_uid) {
        active = &it->second.cells;
    }

    m_suppress_toggle = true;
    for (int r = 0; r < rows; ++r) {
        auto* row_label = new QLabel(QStringLiteral("OUT %1").arg(r));
        row_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_matrix_layout->addWidget(row_label, r + 1, 0);
        for (int c = 0; c < cols; ++c) {
            auto* btn = new QPushButton();
            btn->setCheckable(true);
            btn->setFixedSize(20, 20);
            const auto key = std::make_pair<uint8_t, uint8_t>(
                static_cast<uint8_t>(r), static_cast<uint8_t>(c));
            if (active && active->count(key)) btn->setChecked(true);
            connect(btn, &QPushButton::toggled, this, [this, r, c](bool checked) {
                if (m_suppress_toggle) return;
                on_cell_toggled(static_cast<uint8_t>(r), static_cast<uint8_t>(c), checked);
            });
            m_matrix_layout->addWidget(btn, r + 1, c + 1);
        }
    }
    m_suppress_toggle = false;
}

void RoutingPage::on_cell_toggled(uint8_t out_ch, uint8_t in_ch, bool checked) {
    if (m_source_box->count() == 0 || m_target_box->count() == 0) return;

    const uint16_t src_uid = static_cast<uint16_t>(m_source_box->currentData().toUInt());
    const uint16_t dst_uid = static_cast<uint16_t>(m_target_box->currentData().toUInt());

    auto it = m_routes_by_source.find(src_uid);

    if (checked) {
        // If this source had an active route to a different target, drop
        // it visually + on the wire before adopting the new one. (Single
        // active dest per source.)
        if (it != m_routes_by_source.end() && it->second.target_uid != dst_uid) {
            // Tear down per-cell routes on the OLD target so we don't
            // leave orphan entries behind on it.
            const uint16_t old_dst = it->second.target_uid;
            for (const auto& cell : it->second.cells) {
                send_clear_input_route(old_dst, cell.second);
            }
            m_routes_by_source.erase(it);
            send_clear_audio_dest(src_uid);
            it = m_routes_by_source.end();
        }

        // Column exclusion: a target's input (in_ch) can only be claimed
        // by one source output at a time. If another out_ch in this
        // source's matrix already maps to in_ch, drop it visually + on
        // the wire first. (Engine enforces this independently too, but
        // doing it in the UI keeps the visual honest.)
        if (it != m_routes_by_source.end()) {
            for (auto cit = it->second.cells.begin(); cit != it->second.cells.end(); ) {
                if (cit->second == in_ch && cit->first != out_ch) {
                    // Uncheck the conflicting button without re-firing.
                    if (auto* item = m_matrix_layout->itemAtPosition(cit->first + 1, in_ch + 1)) {
                        if (auto* btn = qobject_cast<QPushButton*>(item->widget())) {
                            m_suppress_toggle = true;
                            btn->setChecked(false);
                            m_suppress_toggle = false;
                        }
                    }
                    send_clear_input_route(dst_uid, cit->second);
                    cit = it->second.cells.erase(cit);
                } else {
                    ++cit;
                }
            }
        }

        const bool need_set_dest = (it == m_routes_by_source.end());
        auto& route = m_routes_by_source[src_uid];
        route.target_uid = dst_uid;
        route.cells.insert({out_ch, in_ch});

        if (need_set_dest) {
            send_set_audio_dest(src_uid, dst_uid);
        }
        send_set_input_route(dst_uid, src_uid, out_ch, in_ch);
    } else {
        if (it == m_routes_by_source.end() || it->second.target_uid != dst_uid) return;
        it->second.cells.erase({out_ch, in_ch});
        send_clear_input_route(dst_uid, in_ch);
        if (it->second.cells.empty()) {
            m_routes_by_source.erase(it);
            send_clear_audio_dest(src_uid);
        }
    }
}

void RoutingPage::clear_all_for_current_source() {
    if (m_source_box->count() == 0) return;
    const uint16_t src_uid = static_cast<uint16_t>(m_source_box->currentData().toUInt());
    auto it = m_routes_by_source.find(src_uid);
    if (it == m_routes_by_source.end()) return;

    const uint16_t dst_uid = it->second.target_uid;
    for (const auto& cell : it->second.cells) {
        send_clear_input_route(dst_uid, cell.second);
    }
    m_routes_by_source.erase(it);
    send_clear_audio_dest(src_uid);
    rebuild_matrix();
}

void RoutingPage::send_set_audio_dest(uint16_t src_uid, uint16_t dst_uid) {
    auto* router = m_sm->get_dsp_manager()->get_router();
    if (!router) return;

    ControlQueryPacket query{};
    query.header.type = PacketType::CONTROL_QUERY;
    query.packet_data.qtype = ControlQueryType::SET_AUDIO_DEST;
    query.packet_data.response[0] = static_cast<uint32_t>(dst_uid);
    router->send_control_packet(query, src_uid);
}

void RoutingPage::send_clear_audio_dest(uint16_t src_uid) {
    auto* router = m_sm->get_dsp_manager()->get_router();
    if (!router) return;

    ControlQueryPacket query{};
    query.header.type = PacketType::CONTROL_QUERY;
    query.packet_data.qtype = ControlQueryType::CLEAR_AUDIO_DEST;
    router->send_control_packet(query, src_uid);
}

void RoutingPage::send_set_input_route(uint16_t dst_uid, uint16_t src_uid,
                                       uint8_t src_ch, uint8_t dest_pipe) {
    auto* router = m_sm->get_dsp_manager()->get_router();
    if (!router) return;

    ControlQueryPacket query{};
    query.header.type = PacketType::CONTROL_QUERY;
    query.packet_data.qtype = ControlQueryType::SET_INPUT_ROUTE;
    query.packet_data.response[0] = static_cast<uint32_t>(src_uid)
                                  | (static_cast<uint32_t>(src_ch) << 16);
    query.packet_data.response[1] = static_cast<uint32_t>(dest_pipe);
    router->send_control_packet(query, dst_uid);
}

void RoutingPage::send_clear_input_route(uint16_t dst_uid, uint8_t dest_pipe) {
    auto* router = m_sm->get_dsp_manager()->get_router();
    if (!router) return;

    ControlQueryPacket query{};
    query.header.type = PacketType::CONTROL_QUERY;
    query.packet_data.qtype = ControlQueryType::CLEAR_INPUT_ROUTE;
    query.packet_data.response[1] = static_cast<uint32_t>(dest_pipe);
    router->send_control_packet(query, dst_uid);
}
