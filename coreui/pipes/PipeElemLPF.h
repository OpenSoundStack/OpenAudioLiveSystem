// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#ifndef PIPEELEMLPF_H
#define PIPEELEMLPF_H

#include "coreui/core/PipeDesc.h"
#include "coreui/ui/VizUtils.h"
#include "coreui/core/PipeDesc.h"
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
