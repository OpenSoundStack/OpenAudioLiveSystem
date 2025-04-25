#include "PipeElemLPF.h"

PipeElemLPF::PipeElemLPF(float cutoff) : PipeElemDesc(nullptr) {
    m_cutoff = cutoff;
}

void PipeElemLPF::set_cutoff(float cutoff) {
    m_cutoff = cutoff;
}

void PipeElemLPF::render_elem(QRect zone, QPainter *painter) {
    //painter->fillRect(zone, Qt::gray);
    painter->drawRoundedRect(zone, 5.0f, 5.0f);
}
