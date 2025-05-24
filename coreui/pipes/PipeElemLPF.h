#ifndef PIPEELEMLPF_H
#define PIPEELEMLPF_H

#include "../core/PipeDesc.h"
#include "coreui/ui/VizUtils.h"

#include <QPainterPath>

class PipeElemLPF : public PipeElemDesc {
public:
    PipeElemLPF(float cutoff);

    void set_cutoff(float cutoff);
    void render_elem(QRect zone, QPainter *painter) override;
private:
    float m_cutoff;

    std::vector<float> m_transfer_render;
};

#endif //PIPEELEMLPF_H
