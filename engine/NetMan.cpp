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

#include "NetMan.h"

NetMan::NetMan(AudioPlumber* plumber) {
    m_plumber = plumber;
}

NetMan::~NetMan() {

}

bool NetMan::init_netman(const std::string& iface) {
    m_pconf = PeerConf{};
    m_pconf.dev_type = DeviceType::AUDIO_DSP;
    m_pconf.sample_rate = SamplingRate::SAMPLING_96K;
    m_pconf.topo = NodeTopology{0, 0, 64, 0xFFFFFFFFFFFFFFFF};
    m_pconf.uid = 100;
    m_pconf.iface = iface;
    m_pconf.ck_type = CKTYPE_MASTER;

    const char dname[32] = "OALS Audio DSP";
    memcpy(m_pconf.dev_name, dname, 32);

    m_nmapper = std::make_shared<NetworkMapper>(m_pconf);
    if (!m_nmapper->init_mapper(m_pconf.iface)) {
        std::cerr << LOG_PREFIX << "Failed to init Network Mapper." << std::endl;
        return false;
    }
    m_nmapper->launch_mapping_process();

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
