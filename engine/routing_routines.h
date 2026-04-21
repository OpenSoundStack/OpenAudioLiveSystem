// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef ROUTING_ROUTINES_H
#define ROUTING_ROUTINES_H

#include "log.h"
#include "AudioEngine.h"
#include "piping/AudioPlumber.h"

#include "OpenAudioNetwork/common/AudioRouter.h"

bool control_pipe_create_routing(
    AudioEngine& engine,
    AudioPlumber& plumber,
    AudioRouter& router,
    ControlPipeCreatePacket& pck,
    LowLatHeader& llhdr
);

void reset_dsp_alloc(AudioEngine& engine, AudioRouter& router, LowLatHeader& llhdr);

#endif //ROUTING_ROUTINES_H
