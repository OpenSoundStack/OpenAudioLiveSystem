#ifndef LEVELMEASUREPIPE_H
#define LEVELMEASUREPIPE_H

#include <list>

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

    std::list<float> m_rms_buffer;
    int m_value_counter;
    float m_sum;
};



#endif //LEVELMEASUREPIPE_H
