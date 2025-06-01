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
