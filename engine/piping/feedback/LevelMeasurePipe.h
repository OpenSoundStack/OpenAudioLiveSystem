#ifndef LEVELMEASUREPIPE_H
#define LEVELMEASUREPIPE_H

#include <list>
#include <cmath>

#include "engine/piping/AudioPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/packet_structs.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"

class LevelMeasurePipe : public AudioPipe {
public:
    LevelMeasurePipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper);
    virtual ~LevelMeasurePipe() = default;

    float process_sample(float sample) override;
    void feedback_send(float db_level);
private:
    AudioRouter* m_router;
    std::shared_ptr<NetworkMapper> m_nmapper;

    std::list<float> m_rms_buffer;
    int m_value_counter;
    float m_sum;
};



#endif //LEVELMEASUREPIPE_H
