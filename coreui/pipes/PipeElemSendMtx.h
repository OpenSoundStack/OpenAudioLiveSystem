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

#ifndef PIPEELEMOUTMTX_H
#define PIPEELEMOUTMTX_H

#include "coreui/core/PipeDesc.h"
#include "ui/FaderSendMtx.h"
#include "coreui/ui/PipeVisualizer.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

// Forward decl
class ShowManager;

class PipeElemSendMtx : public PipeElemDesc {
public:
    PipeElemSendMtx(ShowManager* sm, AudioRouter* router);
    ~PipeElemSendMtx() override = default;

    void render_elem(QRect zone, QPainter *painter) override;

private:
    void find_buses();
    bool is_bus(PipeDesc* desc);

    ShowManager* m_sm;
    QList<PipeVisualizer*> m_buses;

    FaderSendMtx* m_fader_mtx;
};

#endif //PIPEELEMOUTMTX_H
