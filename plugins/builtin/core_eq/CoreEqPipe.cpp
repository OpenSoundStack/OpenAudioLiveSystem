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

#include "CoreEqPipe.h"

CoreEqPipe::CoreEqPipe() : m_peak(1000.0f, 2.0f, 0, 96000.0f) {

}

float CoreEqPipe::process_sample(float sample) {
    return m_peak.push_sample(sample);
}

void CoreEqPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        PeakFilterData pfd{};
        memcpy(&pfd, pck.packet_data.data, sizeof(PeakFilterData));

        m_peak.set_gain(pfd.gain);
        m_peak.set_cutoff(pfd.fc);
    }
}
