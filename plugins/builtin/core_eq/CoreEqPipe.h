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

#ifndef COREEQPIPE_H
#define COREEQPIPE_H

#include <iostream>

#include "plugins/loader/AudioPipe.h"
#include "OpenDSP/src/filter/audio/peak.h"

#include "common.h"

class CoreEqPipe : public AudioPipe {
public:
    CoreEqPipe();
    ~CoreEqPipe() override = default;

protected:
    float process_sample(float sample) override;
    void apply_control(ControlPacket &pck) override;

private:
    void init_filters();

    std::array<PeakFilter, 6> m_peaks;
};



#endif //COREEQPIPE_H
