// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PIPEELEMOUTMTX_H
#define PIPEELEMOUTMTX_H

#include "plugins/loader/PipeDesc.h"
#include "plugins/loader/ElemControlData.h"

#include "coreui/ui/PipeVisualizer.h"
#include "ui/FaderSendMtx.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

struct FaderControlFrame {
    uint8_t channel;
    uint16_t host;
    float level;
};

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

    ControlPacket construct_fader_packet(FaderControlFrame& control_frame);

    ShowManager* m_sm;
    QList<PipeVisualizer*> m_buses;

    FaderSendMtx* m_fader_mtx;
};

#endif //PIPEELEMOUTMTX_H
