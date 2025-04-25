#ifndef PIPEELEMLPF_H
#define PIPEELEMLPF_H

#include "PipeDesc.h"

class PipeElemLPF : public PipeElemDesc {
public:
    PipeElemLPF(float cutoff);

    void set_cutoff(float cutoff);
    void render_elem(QRect zone, QPainter *painter) override;
private:
    float m_cutoff;
};

#endif //PIPEELEMLPF_H
