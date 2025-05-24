#ifndef PIPEELEMHPF_H
#define PIPEELEMHPF_H

#include "../core/PipeDesc.h"
#include "coreui/ui/VizUtils.h"

#include <QPainterPath>

class PipeElemHPF : public PipeElemDesc {
public:
    PipeElemHPF(float cutoff);
    ~PipeElemHPF() override = default;

    void set_cutoff(float cutoff);
    void render_elem(QRect zone, QPainter *painter) override;
private:
    float m_cutoff;
};



#endif //PIPEELEMHPF_H
