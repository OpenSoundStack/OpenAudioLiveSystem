#ifndef PIPEELEMHPF_H
#define PIPEELEMHPF_H

#include "../core/PipeDesc.h"
#include "../core/ElemControlData.h"

#include "coreui/ui/VizUtils.h"
#include "ui/FilterVizHPF.h"

#include <QPainterPath>

class PipeElemHPF : public PipeElemDesc {
public:
    PipeElemHPF(AudioRouter* router, float cutoff);
    ~PipeElemHPF() override = default;

    void set_cutoff(float cutoff);
    void render_elem(QRect zone, QPainter *painter) override;
private:
    float m_cutoff;

    std::shared_ptr<GenericElemControlData<float>> m_cutoff_control;
};



#endif //PIPEELEMHPF_H
