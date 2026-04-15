// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef GENERICVIZHANDLE_H
#define GENERICVIZHANDLE_H

#include <cstdint>

struct GenericHandleData {
    bool pressed;
    bool hovered;

    uint32_t hdl_color;
};

#endif //GENERICVIZHANDLE_H
