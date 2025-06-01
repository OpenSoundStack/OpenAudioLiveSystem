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

#include "PipeElemNoEdit.h"

PipeElemNoEdit::PipeElemNoEdit(AudioRouter* router, QString block_name) : PipeElemDesc(router) {
    m_controls = nullptr;
    m_block_name = block_name;

    setFixedHeight(20);
}

void PipeElemNoEdit::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);
    painter->drawText(zone, Qt::AlignCenter, m_block_name);
    draw_frame(painter, zone);
}
