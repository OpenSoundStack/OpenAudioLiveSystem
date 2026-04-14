// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "DSPManager.h"

#include <utility>

DSPManager::DSPManager(std::shared_ptr<NetworkMapper> nmapper) : QObject(nullptr) {
    m_nmapper = std::move(nmapper);
    m_pid_tracker = 0;
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

                // Finding pending pipe
                if (m_pending_desc.find(pck.packet_data.pid) == m_pending_desc.end()) {
                    std::cerr << "DSP returned success for pid = " << (int)pck.packet_data.pid << " but is not is the pending list." << std::endl;
                    return;
                }

                PendingPipe& ppipe = m_pending_desc[pck.packet_data.pid];
                ppipe.channel = pck.packet_data.channel;
                ppipe.host = hdr.sender_uid;

                if (!ppipe.ui_already_exists) {
                    emit ui_add_pipe(ppipe);
                } else {
                    emit synced_to_dsp(ppipe.pid);
                }

                m_pending_desc.erase(ppipe.pid);
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

bool DSPManager::sync_pipe_to_dsp(std::vector<std::string> pipeline, uint16_t pid) {
    int packet_count = pipeline.size();
    int seq_index = 0;

    auto dsp_id = m_nmapper->find_free_dsp();
    if (!dsp_id.has_value()) {
        std::cerr << "No free DSP found. Channel will be marked as virtual." << std::endl;
        return false;
    }

    for (auto& pipe_elem : pipeline) {
        ControlPipeCreatePacket pck{};
        pck.header.type = PacketType::CONTROL_CREATE;
        pck.packet_data.channel = 0; // Unused
        pck.packet_data.seq = seq_index;
        pck.packet_data.seq_max = packet_count;
        pck.packet_data.pid = pid;
        pck.packet_data.stack_position = seq_index;
        memcpy(pck.packet_data.elem_type, pipe_elem.data(), pipe_elem.size());

        m_router->send_control_packet(pck, dsp_id.value());

        seq_index++;
    }

    return true;
}

void DSPManager::add_pipeline_to_sync_queue(const std::vector<std::string>& pipeline, PipeDesc* pdesc, const QString& pipe_name) {
    m_dsp_sync_queue.enqueue({pipeline, {pdesc, pipe_name, 0, 0, false, false, gen_pid()}});
}

void DSPManager::sync_queue_to_dsp() {
    for (int i = 0; i < m_dsp_sync_queue.size(); i++) {
        auto pending_pipe = m_dsp_sync_queue.dequeue();

        if (!sync_pipe_to_dsp(pending_pipe.first, pending_pipe.second.pid)) {
            pending_pipe.second.unsynced = true;

            if (!pending_pipe.second.ui_already_exists) {
                pending_pipe.second.ui_already_exists = true;
                emit ui_add_pipe(pending_pipe.second);
            }

            // If creation failed, requeue
            m_dsp_sync_queue.enqueue(pending_pipe);
        } else {
            m_pending_desc[pending_pipe.second.pid] = std::move(pending_pipe.second);
        }
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

uint16_t DSPManager::gen_pid() {
    m_pid_tracker++;
    return m_pid_tracker;
}
