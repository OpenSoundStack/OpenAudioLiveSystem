// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "coreui/ui/PipeVisualizer.h"
#include "coreui/ui/SignalWindow.h"

#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

#include "../pipes/PipeElemAudioIn.h"
#include "../pipes/PipeElemHPF.h"
#include "../pipes/PipeElemLPF.h"
#include "../pipes/PipeElemNoEdit.h"
#include "../pipes/PipeElemSendMtx.h"
#include "../pipes/PipeElemAudioInMtx.h"

#include "AudioRouterQt.h"
#include "NetworkConfig.h"
#include "DSPManager.h"

#include "plugins/loader/PluginLoader.h"

#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qqueue.h>
#include <QTimer>
#include <QElapsedTimer>

#include <unordered_map>
#include <array>

class ShowManager : public QObject {

    Q_OBJECT

public:
    ShowManager();
    ~ShowManager() override;

    bool init_console(SignalWindow* sw);

    // Set before init_console: if true, the autoconfigurator clears any
    // persisted UID before deriving a fresh one.
    void set_renumber(bool r) { m_renumber = r; }

    void add_pipe(PipeDesc *pipe_desc, QString pipe_name, uint8_t channel, uint16_t host, uint16_t pid,
                  bool unsynced = false);
    void update_page(SignalWindow* swin);

    void load_pipe_config();
    void load_console_config();

    void load_builtin_pipe_types(AudioRouter* router);
    void load_external_plugins();

    DSPManager* get_dsp_manager();
    std::shared_ptr<NetworkMapper> get_network_mapper() const { return m_nmapper; }
    QList<PipeVisualizer*> get_show();

    void new_show(SignalWindow* sw);

signals:
    void elem_control_selected(QWidget* widget, QString pipe_name);
    void pipe_added(PipeVisualizer* desc);

    void peer_change(QString peer_name, int peer_id, bool state);

private:
    void update_pipe_meter_level(const ControlPacket& data);
    void send_to_elem(const ControlPacket& data);
    void mark_pipe_synced(uint16_t pid);

    // For a freshly-created bus (a pipe whose first element is an input
    // matrix), tell the host engine to feed any send-mtx traffic that
    // targets (engine_uid, bus_channel) into the bus's local pipe. This
    // restores the implicit "bus at channel N receives sends with
    // packet.channel=N" behaviour that disappeared when feed_pipe
    // started consulting an explicit route table.
    void auto_route_bus_if_needed(PipeDesc* desc, uint8_t channel, uint16_t host);

    void tick_meter_decay();

    static constexpr int METER_CHANNELS = 64;
    static constexpr float METER_FLOOR_DB = -60.0f;
    // After this gap with no fresh meter packet, decay the visual level
    // toward METER_FLOOR_DB. ~100 ms is fast enough that the user sees
    // "input went quiet" instantly without flickering on a single
    // dropped packet (meters come in at ~125 Hz from the engine).
    static constexpr qint64 METER_STALE_MS = 100;
    // Decay step per tick. 50 ms tick * 1.2 dB/tick = ~24 dB/s; fully
    // collapses from 0 dB to floor in ~2.5 s.
    static constexpr float METER_DECAY_PER_TICK_DB = 1.2f;

    QList<PipeVisualizer*> m_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
    NetworkConfig m_netconfig;
    bool m_renumber = false;

    DSPManager* m_dsp_manager = nullptr;
    std::shared_ptr<PluginLoader> m_plugin_loader;

    // Per-channel meter decay state. m_last_meter_value_db keeps the
    // most-recent value (also used as the decay origin); m_last_meter_ms
    // is monotonic ms timestamp of the last packet for that channel; the
    // QTimer fires every METER_TICK_MS to push the displayed level down
    // when no packet has arrived recently.
    QTimer m_meter_decay_timer;
    QElapsedTimer m_meter_clock;
    std::array<qint64, METER_CHANNELS> m_last_meter_ms{};
    std::array<float, METER_CHANNELS>  m_last_meter_value_db{};
};



#endif //SHOWMANAGER_H
