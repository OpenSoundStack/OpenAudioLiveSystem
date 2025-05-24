#ifndef LEVELMEASUREPIPE_H
#define LEVELMEASUREPIPE_H

#include <iostream>

#include "common/AudioPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/packet_structs.h"

class LevelMeasurePipe : public AudioPipe {
public:
    LevelMeasurePipe(AudioRouter* router);
    virtual ~LevelMeasurePipe() = default;

    float process_sample(float sample) override;
    void feedback_send(float db_level);
private:
    AudioRouter* m_router;
    float m_sum;
};



#endif //LEVELMEASUREPIPE_H
