// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#include "CoreCompElem.h"

CoreCompElem::CoreCompElem(AudioRouter *router) : PipeElemDesc(router) {

}

void CoreCompElem::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);
    draw_frame(painter, zone);
}
