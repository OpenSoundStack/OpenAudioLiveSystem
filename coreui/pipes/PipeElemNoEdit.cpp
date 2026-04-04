// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

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
