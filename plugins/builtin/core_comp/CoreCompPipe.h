// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_CORECOMPPIPE_H
#define OALIVESYSTEM_CORECOMPPIPE_H

#include "plugins/loader/AudioPipe.h"

#include "OpenDSP/src/dynamics/dynamics.h"

#include "CompParams.h"

class CoreCompPipe : public AudioPipe {
public:
    CoreCompPipe();
    ~CoreCompPipe() override = default;

    float transfer_function(float level_db) const;

protected:
    float process_sample(float sample) override;
    void apply_control(ControlPacket &pck) override;

private:
    void update_time_params();

    std::unique_ptr<Dynamics> m_dynproc;

    float m_threshold_db;
    float m_ratio_db;
    float m_makeup_gain_lin;

    int m_attack_ms;
    int m_release_ms;
    int m_hold_ms;
};



#endif //OALIVESYSTEM_CORECOMPPIPE_H
