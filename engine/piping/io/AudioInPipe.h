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

#ifndef AUDIOINPIPE_H
#define AUDIOINPIPE_H

#include "plugins/loader/AudioPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/packet_structs.h"

struct GainTrim {
    float gain;
    float trim;
} __attribute__((packed));

class AudioInPipe : public AudioPipe {
public:
    AudioInPipe(AudioRouter* router);

    float process_sample(float sample) override;
    void set_gain_lin(float gain);
    void set_trim_lin(float trim);

    void apply_control(ControlPacket &pck) override;
private:
    void construct_hw_packet(uint8_t channel);

    AudioRouter* m_router;

    float m_in_gain;
    float m_in_trim;

    ControlPacket m_hw_control;
};



#endif //AUDIOINPIPE_H
