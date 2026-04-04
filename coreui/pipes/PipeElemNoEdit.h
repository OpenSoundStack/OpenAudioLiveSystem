// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PIPEELEMNOEDIT_H
#define PIPEELEMNOEDIT_H

#include "plugins/loader/PipeDesc.h"
#include "OpenAudioNetwork/common/AudioRouter.h"

class PipeElemNoEdit : public PipeElemDesc {
public:
    PipeElemNoEdit(AudioRouter* router, QString block_name);
    ~PipeElemNoEdit() override = default;

    void render_elem(QRect zone, QPainter *painter) override;

private:
    QString m_block_name;
};



#endif //PIPEELEMNOEDIT_H
