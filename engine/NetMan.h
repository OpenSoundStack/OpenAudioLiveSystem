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

#ifdef OAN_UID_AUTOCONF
#include "OpenAudioNetwork/common/UidStore.h"
#endif

#include <memory>


class NetMan {
public:
    NetMan(AudioPlumber* plumber);
    ~NetMan();

#ifdef OAN_UID_AUTOCONF
    // Init the network manager and run UID autoconfiguration against the
    // given store. Store may be null to skip autoconfig (kept for tests
    // and the flag-off code path that doesn't link this overload).
    bool init_netman(const std::string& iface, IUidStore* uid_store);
#else
    bool init_netman(const std::string& iface);
#endif

    uint16_t committed_uid() const { return m_pconf.uid; }

    void update_netman();
    void start_mapping();

    NodeTopology get_self_topo();
    void update_self_topo(NodeTopology new_topo);

    std::shared_ptr<NetworkMapper> get_net_mapper();

    void clock_master_process();

#ifdef OAN_HOST_BACKENDS
    // Wait up to timeout_ms for a sync packet to arrive, then run the
    // 1 s heartbeat tick. Replaces the busy-loop on the engine's
    // clock_syncer thread when running over host backends — the Linux
    // RT path keeps using clock_master_process directly.
    void clock_wait_or_tick(int timeout_ms);
#endif

private:
    std::shared_ptr<NetworkMapper> m_nmapper;
    PeerConf m_pconf;

    std::unique_ptr<LowLatSocket> m_dsp_control;
    AudioPlumber* m_plumber;

    std::unique_ptr<ClockMaster> m_cm;
};



#endif //NETMAN_H
