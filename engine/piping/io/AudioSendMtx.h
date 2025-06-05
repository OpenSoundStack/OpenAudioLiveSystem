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

#ifndef AUDIOOUTMTX_H
#define AUDIOOUTMTX_H

#include "OpenAudioNetwork/common/AudioRouter.h"

#include "engine/piping/AudioPipe.h"

class AudioSendMtx : public AudioPipe {
public:
    AudioSendMtx(AudioRouter* router);
    ~AudioSendMtx() override = default;

protected:
    float process_sample(float sample) override;
};



#endif //AUDIOOUTMTX_H
