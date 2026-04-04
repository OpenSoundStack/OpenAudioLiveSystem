// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef OALIVESYSTEM_PIPEELEMAUDIOINMTX_H
#define OALIVESYSTEM_PIPEELEMAUDIOINMTX_H

#include "PipeElemNoEdit.h"

class PipeElemAudioInMtx : public PipeElemNoEdit {
public:
    PipeElemAudioInMtx(AudioRouter* router);
    ~PipeElemAudioInMtx() override = default;
};


#endif //OALIVESYSTEM_PIPEELEMAUDIOINMTX_H
