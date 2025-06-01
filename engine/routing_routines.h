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
