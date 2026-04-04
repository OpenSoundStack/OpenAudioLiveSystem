// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PIPEELEMLPF_H
#define PIPEELEMLPF_H

#include "plugins/loader/PipeDesc.h"
#include "plugins/loader/PipeDesc.h"

#include "plugins/loader/ui/VizUtils.h"
#include "coreui/pipes/ui/FilterVizLPF.h"

#include <QPainterPath>

class PipeElemLPF : public PipeElemDesc {
public:
    PipeElemLPF(AudioRouter* router, float cutoff);

    void set_cutoff(float cutoff);
    void render_elem(QRect zone, QPainter *painter) override;
private:
    float m_cutoff;

    std::shared_ptr<GenericElemControlData<float>> m_cutoff_control;
};

#endif //PIPEELEMLPF_H
