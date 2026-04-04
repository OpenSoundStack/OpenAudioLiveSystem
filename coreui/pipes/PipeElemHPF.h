// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PIPEELEMHPF_H
#define PIPEELEMHPF_H

#include "plugins/loader/PipeDesc.h"
#include "plugins/loader/ElemControlData.h"

#include "plugins/loader/ui/VizUtils.h"
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
