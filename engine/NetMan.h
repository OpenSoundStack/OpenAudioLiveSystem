// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef NETMAN_H
#define NETMAN_H

#include "plugins/loader/AudioPipe.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/common/ClockMaster.h"
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
    void start_mapping();

    NodeTopology get_self_topo();
    void update_self_topo(NodeTopology new_topo);

    std::shared_ptr<NetworkMapper> get_net_mapper();

    void clock_master_process();
private:
    std::shared_ptr<NetworkMapper> m_nmapper;
    PeerConf m_pconf;

    std::unique_ptr<LowLatSocket> m_dsp_control;
    AudioPlumber* m_plumber;

    std::unique_ptr<ClockMaster> m_cm;
};



#endif //NETMAN_H
