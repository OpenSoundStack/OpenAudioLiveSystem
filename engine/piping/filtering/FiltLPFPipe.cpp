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

#include "FiltLPFPipe.h"

FiltLPFPipe::FiltLPFPipe() : m_filter(1000.0f, 96000.0f) {

}

float FiltLPFPipe::process_sample(float sample) {
    return m_filter.push_sample(sample);
}

void FiltLPFPipe::set_filter_cutoff(float cutoff) {
    m_filter.set_cutoff(cutoff);
}

