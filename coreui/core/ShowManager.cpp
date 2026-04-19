// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
    infos.ck_type = CKTYPE_NONE;
    memcpy(infos.dev_name, device_name, 32);

    m_nmapper = std::make_shared<NetworkMapper>(infos);
    if (!m_nmapper->init_mapper(infos.iface)) {
        return false;
    }

    std::cout << "Starting netmapper and router processes on interface " << infos.iface << std::endl;

    m_nmapper->set_peer_change_callback([this](PeerInfos& infos, bool peer_state) {
        if (peer_state && infos.peer_data.type == DeviceType::AUDIO_DSP) {
            m_dsp_manager->sync_queue_to_dsp();
            std::cout << "Peer state changed (ID = " << (int)infos.peer_data.self_uid << "), syncing..." << std::endl;
        }

        emit peer_change(QString(infos.peer_data.dev_name), infos.peer_data.self_uid, peer_state);
    });
    m_nmapper->launch_mapping_process();

    m_dsp_manager = new DSPManager(m_nmapper);
    if (!m_dsp_manager->init_dsp_manager(m_netconfig)) {
        std::cerr << "Failed to init DSP Manager" << std::endl;
        return false;
    }

    m_plugin_loader = std::make_shared<PluginLoader>();

    load_builtin_pipe_types(m_dsp_manager->get_router());
    load_external_plugins();

    connect(m_dsp_manager, &DSPManager::ui_add_pipe, this, [this, sw](PendingPipe pipe) {
        add_pipe(pipe.desc, pipe.pipe_name, pipe.channel, pipe.host, pipe.pid, pipe.unsynced);
        update_page(sw);
    });

    connect(m_dsp_manager, &DSPManager::synced_to_dsp, this, [this](uint16_t pid) {
        mark_pipe_synced(pid);
    });

    connect(m_dsp_manager, &DSPManager::control_changed, this, [this](ControlPacket pck) {
        if (pck.packet_data.control_id == 0 && pck.packet_data.control_type == DataTypes::FLOAT) {
            update_pipe_meter_level(pck);
        } else {
            send_to_elem(pck);
        }
    });

    return true;
}

void ShowManager::update_pipe_meter_level(const ControlPacket &data) {
    for (auto& pipe : m_show_content) {
        if (pipe->get_channel() == data.packet_data.channel) {
            float db_level = -60.0f;
            memcpy(&db_level, data.packet_data.data, sizeof(float));

            pipe->set_current_level(db_level);

            break;
        }
    }
}

void ShowManager::send_to_elem(const ControlPacket &data) {
    for (auto& pipe : m_show_content) {
        if (pipe->get_channel() == data.packet_data.channel) {
            pipe->control_to_elem(data);
        }
    }
}

void ShowManager::add_pipe(PipeDesc *desc, QString pipe_name, uint8_t channel, uint16_t host, uint16_t pid,
                           bool unsynced) {
    auto* pipe_viz = new PipeVisualizer{std::move(pipe_name), pid, unsynced, channel};
    m_show_content.append(pipe_viz);

    connect(pipe_viz, &PipeVisualizer::elem_selected, this, [this](PipeDesc* elem, QString selected_pipe_name) {
        QWidget* elem_widget = elem->desc_content->get_controllable_widget();
        emit elem_control_selected(elem_widget, std::move(selected_pipe_name));
    });

    desc->set_pipe_channel(channel, host);
    pipe_viz->set_pipe_content(desc);

    emit pipe_added(pipe_viz);
}

void ShowManager::update_page(SignalWindow *swin) {
    swin->set_page_content(m_show_content);
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

    m_dsp_manager->register_pipe_desc_type("lpf1", [router]() {
        return new PipeElemLPF{router, 1000.0f};
    });

    m_dsp_manager->register_pipe_desc_type("hpf1", [router]() {
        return new PipeElemHPF{router, 100.0f};
    });

    m_dsp_manager->register_pipe_desc_type("dbmeas", [router]() {
        return new PipeElemNoEdit{router, "RMS Meter"};
    });

    m_dsp_manager->register_pipe_desc_type("sendmtx", [router, this]() {
        return new PipeElemSendMtx{this, router};
    });

    m_dsp_manager->register_pipe_desc_type("inmtx", [router]() {
        return new PipeElemAudioInMtx{router};
    });

    m_dsp_manager->register_pipe_desc_type("dirout", [router]() {
        return new PipeElemNoEdit{router, "Direct Out"};
    });
}

void ShowManager::load_external_plugins() {
    std::filesystem::path plugin_root{ENGINE_PLUGIN_SYSLOCATION};

    for (auto& f : std::filesystem::directory_iterator(plugin_root)) {
        auto plugmeta = m_plugin_loader->load_plugin(f.path());
        if (plugmeta.has_value()) {
            std::cout << "Loaded plugin !" << std::endl;

            PluginMeta& meta = plugmeta.value();
            auto piface = meta.plugin_iface;
            AudioRouter* router = m_dsp_manager->get_router();

            m_dsp_manager->register_pipe_desc_type(meta.plugin_name, [piface, router]() {
                return piface->construct_pipe_elem_desc(router);
            });
        }
    }
}

DSPManager *ShowManager::get_dsp_manager() {
    return m_dsp_manager;
}

void ShowManager::new_show(SignalWindow* sw) {
    std::cout << "Resetting DSP for new show..." << std::endl;
    m_dsp_manager->reset_dsp(100);

    // Clear existing pipes
    auto old_show = m_show_content;
    m_show_content.clear();

    update_page(sw);

    for (auto& elem : old_show) {
        delete elem;
    }
}

QList<PipeVisualizer *> ShowManager::get_show() {
    return m_show_content;
}

void ShowManager::mark_pipe_synced(uint16_t pid) {
    for (auto& p : m_show_content) {
        if (p->get_pid() == pid) {
            p->mark_synced();
            return;
        }
    }
}
