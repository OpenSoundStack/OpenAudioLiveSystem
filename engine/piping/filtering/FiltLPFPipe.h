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

#ifndef FILTLPFPIPE_H
#define FILTLPFPIPE_H

#include "plugins/loader/AudioPipe.h"
#include "OpenDSP/src/filter/analog/lowpass.h"

class FiltLPFPipe : public AudioPipe {
public:
    FiltLPFPipe();
    ~FiltLPFPipe() = default;

    void set_filter_cutoff(float cutoff);

protected:
    float process_sample(float sample) override;

private:
    LPF_1ord<float> m_filter;
};



#endif //FILTLPFPIPE_H
