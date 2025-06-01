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

#ifndef PIPEELEMAUDIOIN_H
#define PIPEELEMAUDIOIN_H

#include "../core/PipeDesc.h"
#include "ui/GainTrimUI.h"
#include "coreui/core/ElemControlData.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

#include <QPushButton>

struct GainTrim {
    float gain;
    float trim;
} __attribute__((packed));

class PipeElemAudioIn : public PipeElemDesc {
public:
    PipeElemAudioIn(AudioRouter* router);
    ~PipeElemAudioIn() override = default;

    void render_elem(QRect zone, QPainter *painter) override;

    static float get_db(float lin_val);
    static float get_lin(float db_val);
private:

    AudioRouter* m_router;

    std::shared_ptr<GenericElemControlData<GainTrim>> m_control_data;
};



#endif //PIPEELEMAUDIOIN_H
