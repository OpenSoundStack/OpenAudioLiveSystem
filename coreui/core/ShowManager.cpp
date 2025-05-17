#include "ShowManager.h"

ShowManager::ShowManager() {

}

ShowManager::~ShowManager() {

}

bool ShowManager::init_console() {
    constexpr char device_name[32] = "CORE I Control Surface";

    PeerConf infos{};
    infos.dev_type = DeviceType::CONTROL_SURFACE;
    infos.iface = "virbr0";
    infos.sample_rate = SamplingRate::SAMPLING_96K;
    infos.uid = 200;
    infos.topo.phy_in_count = 0;
    infos.topo.phy_out_count = 0;
    infos.topo.pipes_count = 0;
    memcpy(infos.dev_name, device_name, 32);

    m_nmapper = std::make_shared<NetworkMapper>(infos);
    if (!m_nmapper->init_mapper(infos.iface)) {
        return false;
    }

    m_nmapper->launch_mapping_process();

    return true;
}


void ShowManager::add_pipe() {
    m_ui_show_content.append(new PipeVisualizer{
        (int)m_ui_show_content.size()
    });
}

void ShowManager::update_page(SignalWindow *swin) {
    swin->set_page_content(m_ui_show_content);
}

