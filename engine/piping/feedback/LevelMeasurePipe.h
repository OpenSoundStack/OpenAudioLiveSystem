// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef LEVELMEASUREPIPE_H
#define LEVELMEASUREPIPE_H

#include <list>
#include <cmath>

#include "plugins/loader/AudioPipe.h"

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
