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

#ifndef AUDIODIRECTOUT_H
#define AUDIODIRECTOUT_H

#include "plugins/loader/AudioPipe.h"
#include "OpenAudioNetwork/common/AudioRouter.h"

#include <vector>

struct OutputRoute {
    uint16_t host;
    uint8_t dest_channel;
};

class AudioDirectOut : public AudioPipe {
public:
    AudioDirectOut(AudioRouter* router);
    ~AudioDirectOut() override = default;

    void feed_packet(AudioPacket &pck) override;

private:
    AudioRouter* m_router;
    std::vector<OutputRoute> m_routing_list;
};



#endif //AUDIODIRECTOUT_H
