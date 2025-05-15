#ifndef FILTHPFPIPE_H
#define FILTHPFPIPE_H

#include "OpenAudioNetwork/common/AudioPipe.h"
#include "OpenDSP/src/filter/analog/highpass.h"

class FiltHPFPipe : public AudioPipe {
public:
    FiltHPFPipe();
    ~FiltHPFPipe() = default;

    void set_filter_cutoff(float cutoff);

protected:
    float process_sample(float sample) override;

private:
    HPF_1ord<float> m_filter;
};

#endif //FILTHPFPIPE_H
