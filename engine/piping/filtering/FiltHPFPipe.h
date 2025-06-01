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
