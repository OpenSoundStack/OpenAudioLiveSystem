// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_CORECOMPELEM_H
#define OALIVESYSTEM_CORECOMPELEM_H

#include "plugins/loader/PipeDesc.h"

#include "CoreComp_UI.h"
#include "CompViz.h"

class CoreCompElem : public PipeElemDesc {
public:
    CoreCompElem(AudioRouter* router);
    ~CoreCompElem() override = default;

    void render_elem(QRect zone, QPainter *painter) override;

private:
    float m_threshold;
    float m_ratio;
};



#endif //OALIVESYSTEM_CORECOMPELEM_H
