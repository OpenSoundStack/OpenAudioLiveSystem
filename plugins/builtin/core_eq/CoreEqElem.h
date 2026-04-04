// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef COREEQUI_H
#define COREEQUI_H

#include "plugins/loader/PipeDesc.h"
#include "plugins/loader/ElemControlData.h"

#include "common.h"
#include "CoreEQ_UI.h"

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
    CoreEQ_UI* m_main_ui;
};



#endif //COREEQUI_H
