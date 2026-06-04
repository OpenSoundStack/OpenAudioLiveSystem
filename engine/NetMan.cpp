// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include <iostream>

#include "NetMan.h"

NetMan::NetMan(AudioPlumber* plumber) {
    m_plumber = plumber;
}

NetMan::~NetMan() {

}

bool NetMan::init_netman(const std::string& iface, IUidStore* uid_store) {
    m_pconf = PeerConf{};
    m_pconf.dev_type = DeviceType::AUDIO_DSP;
    m_pconf.sample_rate = SamplingRate::SAMPLING_96K;
    m_pconf.topo = NodeTopology{0, 0, 64, 0xFFFFFFFFFFFFFFFF};
    m_pconf.uid = 0; // 0 = "no hint", let the configurator pick.
    m_pconf.iface = iface;
    m_pconf.ck_type = CKTYPE_MASTER;

    const char dname[32] = "OALS Audio DSP";
    memcpy(m_pconf.dev_name, dname, 32);

    m_nmapper = std::make_shared<NetworkMapper>(m_pconf);
    if (!m_nmapper->init_mapper(m_pconf.iface)) {
        std::cerr << LOG_PREFIX << "Failed to init Network Mapper." << std::endl;
        return false;
    }

    if (uid_store) {
        uint16_t committed = m_nmapper->autoconfigure_uid(*uid_store);
        if (committed == 0) {
            std::cerr << LOG_PREFIX << "UID autoconfiguration failed." << std::endl;
            return false;
        }
        m_pconf.uid = committed;
    } else {
        // No store: caller (e.g. tests) accepts whatever PeerConf::uid was
        // pre-seeded with. Mirror it back out of the mapper for consistency.
        m_pconf.uid = m_nmapper->committed_uid();
    }

    m_dsp_control = std::make_unique<LowLatSocket>(m_pconf.uid, m_nmapper);
    if (!m_dsp_control->init_socket(m_pconf.iface, EthProtocol::ETH_PROTO_OANCONTROL)) {
        std::cerr << LOG_PREFIX << "Failed to init DSP Control socket." << std::endl;
        return false;
    }

    m_cm = std::make_unique<ClockMaster>(m_pconf.uid, iface, m_nmapper);

    return true;
}

void NetMan::update_netman() {

}

std::shared_ptr<NetworkMapper> NetMan::get_net_mapper() {
    return m_nmapper;
}

NodeTopology NetMan::get_self_topo() {
    return m_pconf.topo;
}

void NetMan::update_self_topo(NodeTopology new_topo) {
    m_pconf.topo = new_topo;
    m_nmapper->update_resource_mapping(new_topo);
}

#ifdef OAN_HOST_BACKENDS
void NetMan::clock_wait_or_tick(int timeout_ms) {
    // Drain the sync poll-spin: block until the sync socket has data
    // (or timeout), then let clock_master_process handle the 1 s
    // heartbeat + recv. Result is ignored — recv inside
    // clock_master_process is still non-blocking and returns 0 cleanly
    // on spurious wakeups / timeouts.
    m_cm->wait_sync_readable(timeout_ms);
    clock_master_process();
}
#endif

void NetMan::clock_master_process() {
    constexpr uint64_t sync_interval = 1000000;
    static uint64_t last_sync = NetworkMapper::local_now_us();

    auto now = NetworkMapper::local_now_us();
    if (now - last_sync > sync_interval) {
        m_cm->begin_sync_process();

        last_sync = now;
    }

    m_cm->sync_process();
}

void NetMan::start_mapping() {
    m_nmapper->launch_mapping_process();
}
