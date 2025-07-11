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

#ifndef DSPMANAGER_H
#define DSPMANAGER_H

#include <qobject.h>
#include <qqueue.h>

#include "AudioRouterQt.h"
#include "../../plugins/loader/PipeDesc.h"
#include "NetworkConfig.h"

#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

struct PendingPipe {
    PipeDesc* desc;
    QString pipe_name;
    uint8_t channel;
    uint16_t host;
};

class DSPManager : public QObject {

    Q_OBJECT

public:
    DSPManager(std::shared_ptr<NetworkMapper> nmapper);
    ~DSPManager() override = default;

    bool init_dsp_manager(const NetworkConfig& netconfig);

    void add_pipe_template(const std::string& name, std::vector<std::string> content);
    std::vector<std::string> get_pipe_templates();
    std::optional<std::vector<std::string>> get_template_components(const std::string& name);

    std::optional<PipeDesc*> construct_pipeline_desc(const std::vector<std::string>& pipeline);
    std::optional<PipeElemDesc*> construct_pipe_elem_desc(const std::string& pipe_type);
    void register_pipe_desc_type(const std::string& type, std::function<PipeElemDesc*()> callback);

    void sync_pipe_to_dsp(std::vector<std::string> pipeline);
    void sync_queue_to_dsp();
    void add_pipeline_to_sync_queue(const std::vector<std::string>& pipeline, PipeDesc* pdesc, const QString& pipe_name);

    void reset_dsp(uint16_t uid);

    AudioRouter* get_router();
signals:
    void ui_add_pipe(PendingPipe pipe);
    void control_changed(ControlPacket control_data);

private:
    QtWrapper::AudioRouterQt* m_router;
    std::shared_ptr<NetworkMapper> m_nmapper;
    NetworkConfig m_netconfig;

    std::unordered_map<std::string, std::vector<std::string>> m_pipe_templates;
    std::unordered_map<std::string, std::function<PipeElemDesc*()>> m_pipe_desc_builder;

    QQueue<std::pair<std::vector<std::string>, PendingPipe>> m_dsp_sync_queue;
    PendingPipe m_pending_desc;
};



#endif //DSPMANAGER_H
