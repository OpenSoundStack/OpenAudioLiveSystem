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

#ifndef NETMAN_H
#define NETMAN_H

#include "engine/piping/AudioPipe.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"
#include "OpenAudioNetwork/netutils/LowLatSocket.h"

#include "piping/AudioPlumber.h"
#include "log.h"

#include <memory>


class NetMan {
public:
    NetMan(AudioPlumber* plumber);
    ~NetMan();

    bool init_netman(const std::string& iface);
    void update_netman();

    NodeTopology get_self_topo();
    void update_self_topo(NodeTopology new_topo);

    std::shared_ptr<NetworkMapper> get_net_mapper();
private:
    std::shared_ptr<NetworkMapper> m_nmapper;
    PeerConf m_pconf;

    std::unique_ptr<LowLatSocket> m_dsp_control;
    AudioPlumber* m_plumber;
};



#endif //NETMAN_H
