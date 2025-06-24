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

    void add_pipe(PipeDesc* pipe_desc, QString pipe_name, uint8_t channel, uint16_t host);
    void update_page(SignalWindow* swin);

    void load_pipe_config();
    void load_console_config();

    void load_builtin_pipe_types(AudioRouter* router);

    DSPManager* get_dsp_manager();
    QList<PipeVisualizer*> get_show();

    void new_show(SignalWindow* sw);

signals:
    void elem_control_selected(QWidget* widget, QString pipe_name);
    void pipe_added(PipeVisualizer* desc);

    void peer_change(QString peer_name, int peer_id, bool state);

private:
    void update_pipe_meter_level(const ControlPacket& data);

    QList<PipeVisualizer*> m_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
    NetworkConfig m_netconfig;

    DSPManager* m_dsp_manager;
};



#endif //SHOWMANAGER_H
