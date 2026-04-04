// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "PipeElemAudioInMtx.h"

PipeElemAudioInMtx::PipeElemAudioInMtx(AudioRouter* router) : PipeElemNoEdit(router, "In Matrix") {
    m_flags = ElemFlags::ELEM_IS_INPUT_MATRIX;
}
