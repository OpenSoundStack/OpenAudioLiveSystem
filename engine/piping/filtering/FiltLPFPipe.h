// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef FILTLPFPIPE_H
#define FILTLPFPIPE_H

#include "plugins/loader/AudioPipe.h"
#include "OpenDSP/src/filter/analog/lowpass.h"

class FiltLPFPipe : public AudioPipe {
public:
    FiltLPFPipe();
    ~FiltLPFPipe() = default;

    void set_filter_cutoff(float cutoff);

    void apply_control(ControlPacket &pck) override;
protected:
    float process_sample(float sample) override;

private:
    LPF_1ord m_filter;
};



#endif //FILTLPFPIPE_H
