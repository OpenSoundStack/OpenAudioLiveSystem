// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_DEBUGGERWINDOW_H
#define OALIVESYSTEM_DEBUGGERWINDOW_H

#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QGraphicsScene>

#include <OpenAudioNetwork/common/NetworkMapper.h>
#include <OpenAudioNetwork/netutils/LowLatSocket.h>

#include <sndfile.h>

QT_BEGIN_NAMESPACE

namespace Ui {
    class DebuggerWindow;
}

QT_END_NAMESPACE

class DebuggerWindow : public QWidget {
    Q_OBJECT

public:
    explicit DebuggerWindow(QWidget *parent = nullptr);

    ~DebuggerWindow() override;

signals:
    void stats_changed();
    void audio_received(AudioData data);

private:
    void init_network();
    void init_stats();
    void init_scope_scene();

    void update_stats();
    void reset_min_max();

    void scope_render_audio(const AudioData& data);

    Ui::DebuggerWindow *ui;
    QGraphicsScene* m_stream_scope_scene;

    std::shared_ptr<NetworkMapper> m_nmapper;
    std::shared_ptr<LowLatSocket> m_audio_socket;
    PeerConf m_pconf;

    std::list<uint64_t> m_packet_deltas;
    uint64_t m_last_stamp;
    uint64_t m_packet_max_delta;
    uint64_t m_packet_min_delta;
    float m_mean_delta;
    bool m_first_pck_received;

    uint64_t m_rendered_packets_count;
    bool m_is_recording;
    std::vector<AudioData> m_audio_packets;
};


#endif //OALIVESYSTEM_DEBUGGERWINDOW_H