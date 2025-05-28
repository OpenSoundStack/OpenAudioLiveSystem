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
