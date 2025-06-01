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
