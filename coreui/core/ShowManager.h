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

#include <unordered_map>

class ShowManager : public QObject {

    Q_OBJECT

public:
    ShowManager();
    ~ShowManager() override;

    bool init_console(SignalWindow* sw);

    void add_pipe(PipeDesc *pipe_desc, QString pipe_name, uint8_t channel, uint16_t host, uint16_t pid,
                  bool unsynced = false);
    void update_page(SignalWindow* swin);

    void load_pipe_config();
    void load_console_config();

    void load_builtin_pipe_types(AudioRouter* router);
    void load_external_plugins();

    DSPManager* get_dsp_manager();
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

    QList<PipeVisualizer*> m_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
    NetworkConfig m_netconfig;

    DSPManager* m_dsp_manager;
    std::shared_ptr<PluginLoader> m_plugin_loader;
};



#endif //SHOWMANAGER_H
