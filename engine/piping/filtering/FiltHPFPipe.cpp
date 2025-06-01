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

#include "FiltHPFPipe.h"

FiltHPFPipe::FiltHPFPipe() : m_filter(100.0f, 96000.0f) {

}

float FiltHPFPipe::process_sample(float sample) {
    return m_filter.push_sample(sample);
}

void FiltHPFPipe::set_filter_cutoff(float cutoff) {
    m_filter.set_cutoff(cutoff);
}

void FiltHPFPipe::apply_control(ControlPacket &pck) {
    if (pck.packet_data.control_id == 1) {
        float cutoff = 0.0f;
        memcpy(&cutoff, pck.packet_data.data, sizeof(float));

        set_filter_cutoff(cutoff);
    }
}
