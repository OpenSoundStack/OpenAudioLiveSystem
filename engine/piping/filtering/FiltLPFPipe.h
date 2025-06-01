#ifndef FILTLPFPIPE_H
#define FILTLPFPIPE_H

#include "engine/piping/AudioPipe.h"
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
