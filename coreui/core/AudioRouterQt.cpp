// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

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