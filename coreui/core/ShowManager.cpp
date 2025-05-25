#include "ShowManager.h"

#include <utility>

ShowManager::ShowManager() : QObject(nullptr) {
    m_netconfig = NetworkConfig{};
}

ShowManager::~ShowManager() {

}

bool ShowManager::init_console(SignalWindow* sw) {
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

    std::cout << "Starting netmapper and router processes on interface " << infos.iface << std::endl;

    m_nmapper->launch_mapping_process();

    m_dsp_manager = new DSPManager(m_nmapper);
    if (!m_dsp_manager->init_dsp_manager(m_netconfig)) {
        std::cerr << "Failed to init DSP Manager" << std::endl;
        return false;
    }

    load_builtin_pipe_types(m_dsp_manager->get_router());

    connect(m_dsp_manager, &DSPManager::ui_add_pipe, this, [this, sw](PendingPipe pipe) {
        add_pipe(pipe.desc, pipe.pipe_name, pipe.channel);
        update_page(sw);
    });

    connect(m_dsp_manager, &DSPManager::control_changed, this, [this](ControlPacket pck) {
        if (pck.packet_data.control_id == 0 && pck.packet_data.control_type == DataTypes::FLOAT) {
            update_pipe_meter_level(pck);
        }
    });

    return true;
}

void ShowManager::update_pipe_meter_level(const ControlPacket &data) {
    for (auto& pipe : m_ui_show_content) {
        if (pipe->get_channel() == data.packet_data.channel) {
            float db_level = -60.0f;
            memcpy(&db_level, data.packet_data.data, sizeof(float));

            pipe->set_current_level(db_level);

            break;
        }
    }
}

void ShowManager::add_pipe(PipeDesc* desc, QString pipe_name, uint8_t channel) {
    auto* pipe_viz = new PipeVisualizer{std::move(pipe_name), channel};
    m_ui_show_content.append(pipe_viz);

    connect(pipe_viz, &PipeVisualizer::elem_selected, this, [this](PipeDesc* elem, QString selected_pipe_name) {
        QWidget* elem_widget = elem->desc_content->get_controllable_widget();
        emit elem_control_selected(elem_widget, std::move(selected_pipe_name));
    });

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
        m_dsp_manager->add_pipe_template("Default", {"audioin", "dbmeas", "hpf"});

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

        m_dsp_manager->add_pipe_template(name, std::move(pipeline));

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

void ShowManager::load_builtin_pipe_types(AudioRouter* router) {
    m_dsp_manager->register_pipe_desc_type("audioin", [router]() {
        return new PipeElemAudioIn{router};
    });

    m_dsp_manager->register_pipe_desc_type("lpf1", []() {
        return new PipeElemLPF{100.0f};
    });

    m_dsp_manager->register_pipe_desc_type("hpf1", []() {
        return new PipeElemHPF{100.0f};
    });

    m_dsp_manager->register_pipe_desc_type("dbmeas", []() {
        return new PipeElemNoEdit{"RMS Meter"};
    });
}

DSPManager *ShowManager::get_dsp_manager() {
    return m_dsp_manager;
}

void ShowManager::new_show(SignalWindow* sw) {
    std::cout << "Resetting DSP for new show..." << std::endl;
    m_dsp_manager->reset_dsp(100);

    // Clear existing pipes
    auto old_show = m_ui_show_content;
    m_ui_show_content.clear();

    update_page(sw);

    for (auto& elem : old_show) {
        delete elem;
    }
}

