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

#ifndef COREEQUI_H
#define COREEQUI_H

#include "plugins/loader/PipeDesc.h"
#include "plugins/loader/ElemControlData.h"

#include "common.h"
#include "CoreEqControlUI.h"

class CoreEqElem : public PipeElemDesc {
public:
    CoreEqElem(AudioRouter* router);
    ~CoreEqElem() override = default;

    void render_elem(QRect zone, QPainter *painter) override;
private:
    void init_filters();
    void update_curve();

    std::array<std::shared_ptr<GenericElemControlData<FilterParams>>, 6> m_filters;
    std::vector<QPointF> m_eq_curve;

    CoreEqControlUI* m_control_ui;
};



#endif //COREEQUI_H
