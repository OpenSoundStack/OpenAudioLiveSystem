// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PIPEELEMAUDIOIN_H
#define PIPEELEMAUDIOIN_H

#include "plugins/loader/PipeDesc.h"
#include "plugins/loader/ElemControlData.h"

#include "ui/GainTrimUI.h"

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
