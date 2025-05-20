#include "ShowManager.h"

ShowManager::ShowManager() : QObject(nullptr) {
    m_netconfig = NetworkConfig{};
}

ShowManager::~ShowManager() {

}

bool ShowManager::init_console() {
    constexpr char device_name[32] = "CORE I Control Surface";

    PeerConf infos{};
    infos.dev_type = DeviceType::CONTROL_SURFACE;
    infos.iface = m_netconfig.eth_interface;
    infos.sample_rate = SamplingRate::SAMPLING_96K;
    infos.uid = m_netconfig.uid;
    infos.topo.phy_in_count = 0;
    infos.topo.phy_out_count = 0;
    infos.topo.pipes_count = 0;
    memcpy(infos.dev_name, device_name, 32);

    m_nmapper = std::make_shared<NetworkMapper>(infos);
    if (!m_nmapper->init_mapper(infos.iface)) {
        return false;
    }

    m_router = new QtWrapper::AudioRouterQt{m_netconfig.uid};
    if (!m_router->init_audio_router(m_netconfig.eth_interface, m_nmapper)) {
        return false;
    }

    std::cout << "Starting netmapper and router processes on interface " << infos.iface << std::endl;

    m_nmapper->launch_mapping_process();
    load_builtin_pipe_types();

    connect(
        m_router, &QtWrapper::AudioRouterQt::control_response_received,
        this, [this](ControlResponsePacket pck, LowLatHeader hdr) {
            if (pck.packet_data.channel == m_last_pending_channel) {
                // Updating temporarily local memory with the new resource map for queued channels, that will be created
                // before next NetworkMapper natural update
                auto dev_mapping = m_nmapper->get_device_topo(hdr.sender_uid).value();
                dev_mapping.pipe_resmap = pck.packet_data.resource_map;
                m_nmapper->update_peer_resource_mapping(dev_mapping, hdr.sender_uid);

                // Syncing last pending pipes to DSP
                sync_queue_to_dsp();
            }

            // Previous channel error management
            if (pck.packet_data.channel == m_last_pending_channel && pck.packet_data.response == ControlResponseCode::CREATE_OK) {
                std::cout << "Successfully mapped pipe on DSP channel " << (int)pck.packet_data.channel << std::endl;
            } else if (pck.packet_data.response != ControlResponseCode::CREATE_OK) {
                std::cerr << "Failed to map pipe on DSP channel " << (int)pck.packet_data.channel;
                std::cerr << " Error message : " << pck.packet_data.err_msg << std::endl;
            }
        }
    );

    return true;
}

void ShowManager::add_pipe(PipeDesc* desc, QString pipe_name) {
    auto* pipe_viz = new PipeVisualizer{pipe_name};
    m_ui_show_content.append(pipe_viz);

    pipe_viz->set_pipe_content(desc);
}

void ShowManager::update_page(SignalWindow *swin) {
    swin->set_page_content(m_ui_show_content);
}

void ShowManager::load_pipe_config() {
    QFile config_file = QFile{"surface_config/pipes.json"};

    if (!config_file.open(QIODeviceBase::ReadOnly)) {
        std::cerr << "Failed to open pipes.json config file" << std::endl;
        std::cerr << "Using default config." << std::endl;

        // Default config for pipeline
        m_pipe_templates["Default"] = {"audioin", "dbmeas", "hpf"};

        return;
    }

    auto doc = QJsonDocument::fromJson(config_file.readAll());
    QJsonArray root = doc["templates"].toArray();

    for (auto preset : root) {
        QJsonObject preset_obj = preset.toObject();

        std::string name = preset_obj["name"].toString().toStdString();
        std::vector<std::string> pipeline = {};

        for (auto elem : preset_obj["components"].toArray()) {
            pipeline.push_back(elem.toString().toStdString());
        }

        m_pipe_templates[name] = std::move(pipeline);

        std::cout << "Loading pipeline template : " << name << std::endl;
    }
}

void ShowManager::load_console_config() {
    QFile config_file = QFile{"surface_config/surface.json"};

    if (!config_file.open(QIODeviceBase::ReadOnly)) {
        std::cerr << "Failed to open surface.json config file" << std::endl;
        std::cerr << "Using default config (iface = lo, uid = 200)" << std::endl;

        NetworkConfig netcfg{};
        netcfg.eth_interface = "lo";
        netcfg.uid = 200;

        m_netconfig = std::move(netcfg);
    }

    auto doc = QJsonDocument::fromJson(config_file.readAll());
    QJsonObject net_root = doc["network"].toObject();

    NetworkConfig netcfg{};
    netcfg.eth_interface = net_root["eth_interface"].toString("lo").toStdString();
    netcfg.uid = net_root["uid"].toInt(200);

    m_netconfig = std::move(netcfg);
}

void ShowManager::load_builtin_pipe_types() {
    register_pipe_desc_type("audioin", []() {
        return new PipeElemAudioIn{};
    });

    register_pipe_desc_type("lpf1", []() {
        return new PipeElemLPF{100.0f};
    });

    register_pipe_desc_type("hpf1", []() {
        return new PipeElemHPF{100.0f};
    });
}


std::vector<std::string> ShowManager::get_pipe_templates() {
    std::vector<std::string> templates{};

    for (auto& elem : m_pipe_templates) {
        templates.emplace_back(elem.first);
    }

    return templates;
}

std::optional<std::vector<std::string> > ShowManager::get_template_components(const std::string &name) {
    auto elem_found = m_pipe_templates.find(name);
    if (elem_found == m_pipe_templates.end()) {
        return {};
    }

    return elem_found->second;
}

void ShowManager::register_pipe_desc_type(std::string type, std::function<PipeElemDesc *()> callback) {
    m_pipe_desc_builder[type] = callback;
}

std::optional<PipeElemDesc *> ShowManager::construct_pipe_elem_desc(std::string pipe_type) {
    auto found_elem = m_pipe_desc_builder.find(pipe_type);
    if (found_elem == m_pipe_desc_builder.end()) {
        return {};
    }

    auto* pipe_desc = m_pipe_desc_builder[pipe_type]();
    return pipe_desc;
}

std::optional<PipeDesc*> ShowManager::construct_pipeline_desc(std::vector<std::string> pipeline) {
    PipeDesc* root = new PipeDesc{};

    PipeDesc* current_elem = root;

    int index = 0;
    for (auto& pipe_elem : pipeline) {
        auto elem_desc = construct_pipe_elem_desc(pipe_elem);

        if (!elem_desc.has_value()) {
            std::cerr << "Unkown pipe description type " << pipe_elem << std::endl;

            delete root;
            return {};
        } else {
            current_elem->desc_content = elem_desc.value();

            // If we are on the last pipe element, do not add a trailing elem on the list
            if (index != pipeline.size() - 1) {
                current_elem->next_pipe_elem = new PipeDesc{};
                current_elem = current_elem->next_pipe_elem.value();
            }
        }

        index++;
    }

    return root;
}

void ShowManager::sync_pipe_to_dsp(std::vector<std::string> pipeline) {
    int packet_count = pipeline.size();
    int seq_index = 0;

    auto dsp_id = m_nmapper->find_free_dsp();
    if (!dsp_id.has_value()) {
        std::cerr << "No free DSP found. Channel will be marked as virtual." << std::endl;
        return;
    }

    auto new_channel = m_nmapper->first_free_processing_channel(dsp_id.value());
    m_last_pending_channel = new_channel.value();

    for (auto& pipe_elem : pipeline) {
        ControlPipeCreatePacket pck{};
        pck.header.type = PacketType::CONTROL_CREATE;
        pck.packet_data.channel = new_channel.value();
        pck.packet_data.seq = seq_index;
        pck.packet_data.seq_max = packet_count;
        pck.packet_data.stack_position = seq_index;
        memcpy(pck.packet_data.elem_type, pipe_elem.data(), pipe_elem.size());

        m_router->send_control_packet(pck, dsp_id.value());

        seq_index++;
    }
}

void ShowManager::add_pipeline_to_sync_queue(const std::vector<std::string>& pipeline) {
    m_dsp_sync_queue.enqueue(pipeline);
}

void ShowManager::sync_queue_to_dsp() {
    if (!m_dsp_sync_queue.isEmpty()) {
        sync_pipe_to_dsp(m_dsp_sync_queue.dequeue());
    }
}

