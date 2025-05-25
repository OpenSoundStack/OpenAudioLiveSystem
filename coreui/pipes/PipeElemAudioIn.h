#ifndef PIPEELEMAUDIOIN_H
#define PIPEELEMAUDIOIN_H

#include "../core/PipeDesc.h"
#include "ui/GainTrimUI.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

#include <QPushButton>

class PipeElemAudioIn : public PipeElemDesc {
public:
    PipeElemAudioIn(AudioRouter* router);
    ~PipeElemAudioIn() = default;

    void set_gain(float gain);
    void set_trim(float trim);

    void render_elem(QRect zone, QPainter *painter) override;

    static float get_db(float lin_val);
    static float get_lin(float db_val);
private:
    void send_control_frame();

    AudioRouter* m_router;

    float m_gain;
    float m_trim;
};



#endif //PIPEELEMAUDIOIN_H
