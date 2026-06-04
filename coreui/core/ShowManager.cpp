// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "ShowManager.h"

#include "OpenAudioNetwork/common/UidStore.h"

#include <qjsonvalue.h>
#include <qsavefile.h>

#include <utility>

namespace {

// Persist the autoconfigured UID into a top-level field of an existing
// QJson document. Atomic via QSaveFile (write-tmp-then-rename).
class QJsonFieldUidStore : public IUidStore {
public:
    QJsonFieldUidStore(QString path, QString field)
        : m_path(std::move(path)), m_field(std::move(field)) {}

    std::optional<uint16_t> load() override {
        QFile f(m_path);
        if (!f.open(QIODevice::ReadOnly)) return std::nullopt;
        QJsonParseError err{};
        auto doc = QJsonDocument::fromJson(f.readAll(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            std::cerr << "QJsonFieldUidStore: parse '" << m_path.toStdString()
                      << "' failed: " << err.errorString().toStdString() << std::endl;
            return std::nullopt;
        }
        auto root = doc.object();
        auto net = root.value("network").toObject();
        auto v = net.value(m_field);
        if (!v.isDouble()) return std::nullopt;
        int i = v.toInt(-1);
        if (i < 0 || i > 0xFFFF) return std::nullopt;
        return static_cast<uint16_t>(i);
    }

    void save(uint16_t uid) override {
        QJsonObject root;
        {
            QFile f(m_path);
            if (f.open(QIODevice::ReadOnly)) {
                auto doc = QJsonDocument::fromJson(f.readAll());
                if (doc.isObject()) root = doc.object();
            }
        }
        QJsonObject net = root.value("network").toObject();
        net.insert(m_field, static_cast<int>(uid));
        root.insert("network", net);

        QSaveFile out(m_path);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            std::cerr << "QJsonFieldUidStore: open '" << m_path.toStdString()
                      << "' for write failed." << std::endl;
            return;
        }
        out.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        if (!out.commit()) {
            std::cerr << "QJsonFieldUidStore: commit '" << m_path.toStdString()
                      << "' failed." << std::endl;
        }
    }

    void clear() override {
        QFile f(m_path);
        if (!f.open(QIODevice::ReadOnly)) return;
        auto doc = QJsonDocument::fromJson(f.readAll());
        f.close();
        if (!doc.isObject()) return;
        auto root = doc.object();
        auto net = root.value("network").toObject();
        if (!net.contains(m_field)) return;
        net.remove(m_field);
        root.insert("network", net);

        QSaveFile out(m_path);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
        out.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        out.commit();
    }

private:
    QString m_path;
    QString m_field;
};

bool is_static_range(uint16_t uid) {
    return uid >= 0xF000 && uid <= 0xFFFE;
}

} // namespace

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
    // Hint = static-range pin only; dynamic-range hints are ignored
    // by the configurator (per design §2.5).
    infos.uid = is_static_range(m_netconfig.hint_uid) ? m_netconfig.hint_uid : 0;
    infos.topo.phy_in_count = 0;
    infos.topo.phy_out_count = 0;
    infos.topo.pipes_count = 0;
    infos.ck_type = CKTYPE_NONE;
    memcpy(infos.dev_name, device_name, 32);

    m_nmapper = std::make_shared<NetworkMapper>(infos);
    if (!m_nmapper->init_mapper(infos.iface)) {
        return false;
    }

    if (!is_static_range(m_netconfig.hint_uid)) {
        // No static pin — run autoconfig, persist to surface.json.
        auto backing = std::make_unique<QJsonFieldUidStore>(
            QStringLiteral("surface_config/surface.json"),
            QStringLiteral("persisted_uid"));
        if (m_renumber) {
            std::cout << "--renumber: clearing persisted UID in surface.json" << std::endl;
            backing->clear();
        }
        EnvOverrideUidStore store{"OAN_PERSISTED_UID", std::move(backing)};
        uint16_t committed = m_nmapper->autoconfigure_uid(store);
        if (committed == 0) {
            std::cerr << "ShowManager: UID autoconfiguration failed." << std::endl;
            return false;
        }
        m_netconfig.uid = committed;
    } else {
        m_netconfig.uid = m_netconfig.hint_uid;
        std::cout << "ShowManager: static-range UID 0x" << std::hex
                  << m_netconfig.hint_uid << std::dec
                  << " pinned; autoconfig skipped." << std::endl;
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

    auto_route_bus_if_needed(desc, channel, host);

    emit pipe_added(pipe_viz);
}

void ShowManager::auto_route_bus_if_needed(PipeDesc* desc, uint8_t channel, uint16_t host) {
    if (!desc || !desc->desc_content) return;
    if (!(desc->desc_content->get_flags() & ElemFlags::ELEM_IS_INPUT_MATRIX)) return;

    // A bus accepts same-engine send-mtx traffic targeting (host,
    // channel). One route entry covers all senders that share that key
    // — the bus's input matrix sums them. (Cross-engine sends would
    // need a separate, manually-added Routing-page entry with the
    // sender's UID.)
    auto* router = m_dsp_manager ? m_dsp_manager->get_router() : nullptr;
    if (!router) return;

    ControlQueryPacket q{};
    q.header.type = PacketType::CONTROL_QUERY;
    q.packet_data.qtype = ControlQueryType::SET_INPUT_ROUTE;
    q.packet_data.response[0] = static_cast<uint32_t>(host)
                              | (static_cast<uint32_t>(channel) << 16);
    q.packet_data.response[1] = static_cast<uint32_t>(channel);
    router->send_control_packet(q, host);
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
        std::cerr << "Using default config (iface = lo, no UID hint)" << std::endl;

        NetworkConfig netcfg{};
        netcfg.eth_interface = "lo";
        netcfg.hint_uid = 0;
        netcfg.persisted_uid = 0;
        m_netconfig = std::move(netcfg);
        return;
    }

    auto doc = QJsonDocument::fromJson(config_file.readAll());
    QJsonObject net_root = doc["network"].toObject();

    NetworkConfig netcfg{};
    netcfg.eth_interface = net_root["eth_interface"].toString("lo").toStdString();
    netcfg.hint_uid = static_cast<uint16_t>(net_root["uid"].toInt(0));
    netcfg.persisted_uid = static_cast<uint16_t>(net_root["persisted_uid"].toInt(0));

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
    auto dsp_id = m_nmapper->find_free_dsp();
    if (dsp_id.has_value()) {
        m_dsp_manager->reset_dsp(dsp_id.value());
    } else {
        std::cerr << "new_show: no DSP discovered yet, skipping reset" << std::endl;
    }

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
