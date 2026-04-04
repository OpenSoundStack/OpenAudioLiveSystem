// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef FILTHPFPIPE_H
#define FILTHPFPIPE_H

#include "plugins/loader/AudioPipe.h"
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
    HPF_1ord m_filter;
};

#endif //FILTHPFPIPE_H
