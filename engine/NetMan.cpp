#include "NetMan.h"

NetMan::NetMan() {

}

NetMan::~NetMan() {

}

void NetMan::init_netman() {
    m_pconf = PeerConf{};
    m_pconf.dev_type = DeviceType::AUDIO_DSP;
    m_pconf.sample_rate = SamplingRate::SAMPLING_96K;
    m_pconf.topo = NodeTopology{0, 0, 64};
    m_pconf.uid = 100;
    m_pconf.iface = "virbr0";

    const char dname[32] = "OALS Audio DSP";
    memcpy(m_pconf.dev_name, dname, 32);

    m_nmapper = std::make_unique<NetworkMapper>(m_pconf);
    m_nmapper->init_mapper(m_pconf.iface);
    m_nmapper->launch_mapping_process();
}
