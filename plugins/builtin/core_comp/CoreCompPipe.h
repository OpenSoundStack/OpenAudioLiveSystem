// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_CORECOMPPIPE_H
#define OALIVESYSTEM_CORECOMPPIPE_H

#include "plugins/loader/AudioPipe.h"

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"

#include "OpenDSP/src/dynamics/dynamics.h"

#include "CompParams.h"

class CoreCompPipe : public AudioPipe {
public:
    CoreCompPipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper);
    ~CoreCompPipe() override = default;

    float transfer_function(float level_db);
    void apply_control(ControlPacket &pck) override;

    void send_feedback(float gain_lin, float enveloppe_db);
protected:
    float process_sample(float sample) override;

private:
    void update_time_params();

    std::unique_ptr<Dynamics> m_dynproc;

    float m_threshold_db;
    float m_ratio_db;
    float m_makeup_gain_lin;

    int m_attack_ms;
    int m_release_ms;
    int m_hold_ms;

    int m_reduction_send_counter;
    float m_current_enveloppe;

    AudioRouter* m_router;
    std::shared_ptr<NetworkMapper> m_nmapper;
};



#endif //OALIVESYSTEM_CORECOMPPIPE_H
