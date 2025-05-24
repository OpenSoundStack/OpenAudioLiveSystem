#ifndef PIPEELEMAUDIOIN_H
#define PIPEELEMAUDIOIN_H

#include "../core/PipeDesc.h"
#include "coreui/ui/VizUtils.h"

#include <QPushButton>

class PipeElemAudioIn : public PipeElemDesc {
public:
    PipeElemAudioIn();
    ~PipeElemAudioIn() = default;

    void set_gain(float gain);
    void set_trim(float trim);

    void render_elem(QRect zone, QPainter *painter) override;

    static float get_db(float lin_val);
private:
    float m_gain;
    float m_trim;
};



#endif //PIPEELEMAUDIOIN_H
