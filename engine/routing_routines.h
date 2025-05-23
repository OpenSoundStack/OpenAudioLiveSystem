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
