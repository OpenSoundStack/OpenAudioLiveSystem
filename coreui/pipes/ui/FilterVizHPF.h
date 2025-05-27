#ifndef FILTERVIZHPF_H
#define FILTERVIZHPF_H

#include "FilterEditBase.h"

class FilterVizHPF : public FilterEditBase {
public:
    FilterVizHPF();
    ~FilterVizHPF() override = default;

    void set_cutoff(float fc) override;
protected:
    void draw_approx_filter(QPainter* painter, QRect zone) override;
};

#endif //FILTERVIZHPF_H
