#ifndef FILTERVIZHPF_H
#define FILTERVIZHPF_H

#include "FilterEditBase.h"

#include "filter/analog/highpass.h"
#include "OpenDSP/src/filter/iirfilter.h"

class FilterVizHPF : public FilterEditBase {
public:
    FilterVizHPF();
    ~FilterVizHPF() override = default;

    void set_cutoff(float fc) override;
protected:
    void calc_filter_mag() override;

private:
    HPF_2ord<float> m_filter;
};



#endif //FILTERVIZHPF_H
