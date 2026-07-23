// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef LEVELMEASUREPIPE_H
#define LEVELMEASUREPIPE_H

#include <queue>
#include <cmath>

#include "plugins/loader/AudioPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/packet_structs.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"

class LevelMeasurePipe : public AudioPipe {
public:
    LevelMeasurePipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper);
    ~LevelMeasurePipe() override = default;

protected:
    void process_samples(std::span<float>& audio_data) override;
    void feedback_send(float db_level);

private:
    AudioRouter* m_router;
    std::shared_ptr<NetworkMapper> m_nmapper;

    std::queue<float> m_rms_buffer;
    int m_value_counter;

    double m_sum;
    double m_y;
    double m_t;
    double m_compensation;
};



#endif //LEVELMEASUREPIPE_H
