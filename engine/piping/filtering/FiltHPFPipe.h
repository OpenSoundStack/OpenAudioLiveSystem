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

#ifndef FILTHPFPIPE_H
#define FILTHPFPIPE_H

#include "engine/piping/AudioPipe.h"
#include "OpenDSP/src/filter/analog/highpass.h"

#include <iostream>

class FiltHPFPipe : public AudioPipe {
public:
    FiltHPFPipe();
    ~FiltHPFPipe() = default;

    void set_filter_cutoff(float cutoff);
    void apply_control(ControlPacket &pck) override;

protected:
    float process_sample(float sample) override;

private:
    HPF_1ord<float> m_filter;
};

#endif //FILTHPFPIPE_H
