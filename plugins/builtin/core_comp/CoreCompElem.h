// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_CORECOMPELEM_H
#define OALIVESYSTEM_CORECOMPELEM_H

#include "plugins/loader/PipeDesc.h"

#include "CoreComp_UI.h"
#include "CompViz.h"
#include "CompParams.h"

class CoreCompElem : public PipeElemDesc {
public:
    CoreCompElem(AudioRouter* router);
    ~CoreCompElem() override = default;

    void render_elem(QRect zone, QPainter *painter) override;

private:
    void setup_dsp_link();

    CompStaticParams m_base_params;
    CompDynamicsParams m_time_params;

    std::shared_ptr<GenericElemControlData<CompStaticParams>> m_static_params;
    std::shared_ptr<GenericElemControlData<CompDynamicsParams>> m_dyn_params;
};



#endif //OALIVESYSTEM_CORECOMPELEM_H
