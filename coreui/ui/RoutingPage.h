// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef ROUTINGPAGE_H
#define ROUTINGPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>

#include <map>
#include <set>
#include <utility>
#include <cstdint>

#include "../core/ShowManager.h"

// One page in the SetupWindow stack. Lets the operator pick a source peer
// (AUDIO_IO_INTERFACE) and a target peer (AUDIO_DSP) from the discovered
// devices, see an output x input matrix, and click a cell to wire the
// source's audio stream to the target.
//
// Current scope (Half 1 of the routing work): the matrix is visual; the
// only wire effect is SET_AUDIO_DEST to the source peer ("send your
// outgoing audio to this UID"). Per-channel matrix routing on the engine
// side does not exist yet — the cell click feels granular but today it
// flips the source's whole stream destination.
//
// Semantics:
// - At most one active destination per source. Clicking a cell in a new
//   (source, target) matrix for a source that already has an active dest
//   moves the dest; the old matrix is cleared.
// - Unchecking the last cell of the current matrix fires CLEAR_AUDIO_DEST.
// - State is in-memory only; lost on UI restart.
class RoutingPage : public QWidget {
    Q_OBJECT
public:
    explicit RoutingPage(ShowManager* sm, QWidget* parent = nullptr);

signals:
    void back_requested();

private:
    void refresh_peer_lists();
    void rebuild_matrix();
    void on_cell_toggled(uint8_t out_ch, uint8_t in_ch, bool checked);
    void send_set_audio_dest(uint16_t src_uid, uint16_t dst_uid);
    void send_clear_audio_dest(uint16_t src_uid);
    void send_set_input_route(uint16_t dst_uid, uint16_t src_uid, uint8_t src_ch, uint8_t dest_pipe);
    void send_clear_input_route(uint16_t dst_uid, uint8_t dest_pipe);
    void clear_all_for_current_source();

    ShowManager* m_sm;

    QComboBox* m_source_box;
    QComboBox* m_target_box;
    QGridLayout* m_matrix_layout;
    QWidget* m_matrix_host;
    QLabel* m_matrix_caption;

    // Per-source state: which (source, target) pair is currently active
    // for this source (at most one), and the set of checked cells in that
    // matrix.
    struct ActiveRoute {
        uint16_t target_uid;
        std::set<std::pair<uint8_t, uint8_t>> cells; // (out_ch, in_ch)
    };
    std::map<uint16_t, ActiveRoute> m_routes_by_source;

    // Suppress on_cell_toggled side effects while we programmatically
    // restore button state in rebuild_matrix.
    bool m_suppress_toggle = false;
};

#endif //ROUTINGPAGE_H
