// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "AudioRouterQt.h"

namespace QtWrapper {
    AudioRouterQt::AudioRouterQt(uint16_t uid) :
        QObject(nullptr), AudioRouter(uid)
    {
        m_polling_thread = QThread::create([this]() {
            while (true) {
                poll_control_packets(false);
            }
        });
    }

    bool AudioRouterQt::init_audio_router(std::string eth_interface, std::shared_ptr<NetworkMapper> nmapper) {
        if (init_router(eth_interface, nmapper)) {
            set_control_callback([this](ControlPacket& pck, LowLatHeader& hdr) {
                emit control_received(pck, hdr);
            });

            set_control_response_callback([this](ControlResponsePacket& pck, LowLatHeader& hdr) {
                emit control_response_received(pck, hdr);
            });

            m_polling_thread->start();

            return true;
        } else {
            return false;
        }
    }

} // QtWrapper