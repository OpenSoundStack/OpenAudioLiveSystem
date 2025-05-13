#include "NetMan.h"

NetMan::NetMan(AudioPlumber* plumber) {
    m_plumber = plumber;
}

NetMan::~NetMan() {

}

bool NetMan::init_netman() {
    m_pconf = PeerConf{};
    m_pconf.dev_type = DeviceType::AUDIO_DSP;
    m_pconf.sample_rate = SamplingRate::SAMPLING_96K;
    m_pconf.topo = NodeTopology{0, 0, 64};
    m_pconf.uid = 100;
    m_pconf.iface = "virbr0";

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

    return true;
}

void NetMan::update_netman() {

}

std::shared_ptr<NetworkMapper> NetMan::get_net_mapper() {
    return m_nmapper;
}
