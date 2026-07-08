// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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

    void apply_control(ControlPacket &pck) override;

protected:
    void process_samples(std::span<float>& audio_data) override;

private:
    void init_filters();

    std::array<PeakFilter, 6> m_peaks;
};



#endif //COREEQPIPE_H
