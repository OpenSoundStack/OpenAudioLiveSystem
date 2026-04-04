// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
