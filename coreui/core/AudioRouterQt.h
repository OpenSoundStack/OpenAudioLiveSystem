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

#ifndef AUDIOROUTERQT_H
#define AUDIOROUTERQT_H

#include <QObject>
#include <QThread>

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/packet_structs.h"

namespace QtWrapper {

class AudioRouterQt : public QObject, public AudioRouter {

    Q_OBJECT

public:
    AudioRouterQt(uint16_t uid);
    ~AudioRouterQt() override = default;

    bool init_audio_router(std::string eth_interface, std::shared_ptr<NetworkMapper> nmapper);
signals:
    void control_received(ControlPacket pck, LowLatHeader hdr);
    void control_response_received(ControlResponsePacket pck, LowLatHeader hdr);

private:
    QThread* m_polling_thread;
};

} // QtWrapper

#endif //AUDIOROUTERQT_H
