// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
