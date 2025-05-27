#include "DSPManager.h"

#include <utility>

DSPManager::DSPManager(std::shared_ptr<NetworkMapper> nmapper) : QObject(nullptr) {
    m_nmapper = std::move(nmapper);
}

bool DSPManager::init_dsp_manager(const NetworkConfig& netconfig) {
    m_netconfig = netconfig;
    m_router = new QtWrapper::AudioRouterQt(netconfig.uid);

    if (!m_router->init_audio_router(m_netconfig.eth_interface, m_nmapper)) {
        std::cerr << "Failed to init audio router" << std::endl;
        return false;
    }

    connect(
        m_router, &QtWrapper::AudioRouterQt::control_response_received,
        this, [this](ControlResponsePacket pck, LowLatHeader hdr) {
            // Previous channel error management
            if (pck.packet_data.response == ControlResponseCode::CREATE_OK) {
                std::cout << "Successfully mapped pipe on DSP (ID = " << (int)hdr.sender_uid << ") channel " << (int)pck.packet_data.channel << std::endl;

                m_pending_desc.channel = pck.packet_data.channel;
                m_pending_desc.host = hdr.sender_uid;
                emit ui_add_pipe(m_pending_desc);
            } else if (pck.packet_data.response & ControlResponseCode::CREATE_ERROR) {
                std::cerr << "Failed to map pipe on DSP (ID = " << (int)hdr.sender_uid << ")";
                std::cerr << " Error message : " << pck.packet_data.err_msg << std::endl;
            } else if (pck.packet_data.response & ControlResponseCode::CONTROL_ACK) {
                std::cout << "Last command executed successfully on DSP (ID = " << (int)hdr.sender_uid << ")" << std::endl;
            }

            // Continue syncing last pending pipes to DSP
            sync_queue_to_dsp();
        }
    );

    connect(m_router, &QtWrapper::AudioRouterQt::control_received, this, [this](ControlPacket pck, LowLatHeader hdr) {
        emit control_changed(pck);
    });

    return true;
}

void DSPManager::add_pipe_template(const std::string& name, std::vector<std::string> content) {
    m_pipe_templates[name] = std::move(content);
}

std::vector<std::string> DSPManager::get_pipe_templates() {
    std::vector<std::string> templates{};

    for (auto& elem : m_pipe_templates) {
        templates.emplace_back(elem.first);
    }

    return templates;
}

std::optional<std::vector<std::string> > DSPManager::get_template_components(const std::string &name) {
    auto elem_found = m_pipe_templates.find(name);
    if (elem_found == m_pipe_templates.end()) {
        return {};
    }

    return elem_found->second;
}

void DSPManager::register_pipe_desc_type(const std::string& type, std::function<PipeElemDesc *()> callback) {
    m_pipe_desc_builder[type] = std::move(callback);
}

std::optional<PipeElemDesc *> DSPManager::construct_pipe_elem_desc(const std::string& pipe_type) {
    auto found_elem = m_pipe_desc_builder.find(pipe_type);
    if (found_elem == m_pipe_desc_builder.end()) {
        std::cerr << "No graphical element for " << pipe_type << std::endl;
        return {};
    }

    auto* pipe_desc = m_pipe_desc_builder[pipe_type]();
    return pipe_desc;
}

std::optional<PipeDesc*> DSPManager::construct_pipeline_desc(const std::vector<std::string>& pipeline) {
    PipeDesc* root = new PipeDesc{};

    PipeDesc* current_elem = root;

    int index = 0;
    for (auto& pipe_elem : pipeline) {
        auto elem_desc = construct_pipe_elem_desc(pipe_elem);

        if (!elem_desc.has_value()) {
            std::cerr << "Unkown pipe description type " << pipe_elem << std::endl;
            std::cerr << "Ignoring..." << std::endl;

            //delete root;
            //return {};
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

    root->index_pipes();
    return root;
}

void DSPManager::sync_pipe_to_dsp(std::vector<std::string> pipeline) {
    int packet_count = pipeline.size();
    int seq_index = 0;

    auto dsp_id = m_nmapper->find_free_dsp();
    if (!dsp_id.has_value()) {
        std::cerr << "No free DSP found. Channel will be marked as virtual." << std::endl;
        return;
    }

    for (auto& pipe_elem : pipeline) {
        ControlPipeCreatePacket pck{};
        pck.header.type = PacketType::CONTROL_CREATE;
        pck.packet_data.channel = 0; // Unused
        pck.packet_data.seq = seq_index;
        pck.packet_data.seq_max = packet_count;
        pck.packet_data.stack_position = seq_index;
        memcpy(pck.packet_data.elem_type, pipe_elem.data(), pipe_elem.size());

        m_router->send_control_packet(pck, dsp_id.value());

        seq_index++;
    }
}

void DSPManager::add_pipeline_to_sync_queue(const std::vector<std::string>& pipeline, PipeDesc* pdesc, const QString& pipe_name) {
    m_dsp_sync_queue.enqueue({pipeline, {pdesc, pipe_name}});
}

void DSPManager::sync_queue_to_dsp() {
    if (!m_dsp_sync_queue.isEmpty()) {
        auto pending_pipe = m_dsp_sync_queue.dequeue();
        m_pending_desc = pending_pipe.second;

        sync_pipe_to_dsp(pending_pipe.first);
    }
}

void DSPManager::reset_dsp(uint16_t uid) {
    ControlQueryPacket query{};
    query.header.type = PacketType::CONTROL_QUERY;
    query.packet_data.qtype = ControlQueryType::PIPE_ALLOC_RESET;

    m_router->send_control_packet(query, uid);
}

AudioRouter *DSPManager::get_router() {
    return m_router;
}

