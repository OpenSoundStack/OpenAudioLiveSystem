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

#include "engine/piping/AudioPipe.h"

struct GainTrim {
    float gain;
    float trim;
} __attribute__((packed));

class AudioInPipe : public AudioPipe {
public:
    AudioInPipe();

    float process_sample(float sample) override;
    void set_gain_lin(float gain);
    void set_trim_lin(float trim);

    void apply_control(ControlPacket &pck) override;
private:
    float m_in_gain;
    float m_in_trim;
};



#endif //AUDIOINPIPE_H
