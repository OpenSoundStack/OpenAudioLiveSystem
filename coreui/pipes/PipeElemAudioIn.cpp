#include "PipeElemAudioIn.h"

PipeElemAudioIn::PipeElemAudioIn() : PipeElemDesc() {
    m_trim = 1.0f;
    m_gain = 1.0f;

    setFixedHeight(60);
}

void PipeElemAudioIn::set_gain(float gain) {
    m_gain = gain;
}

void PipeElemAudioIn::set_trim(float trim) {
    m_trim = trim;
}

void PipeElemAudioIn::render_elem(QRect zone, QPainter *painter) {
    draw_background(painter, zone);

    // Two zones separation
    QRect gain_rect = zone;
    gain_rect.setWidth(zone.width() / 2);

    QRect trim_rect = zone;
    trim_rect.setWidth(zone.width() / 2);
    trim_rect.moveTo(QPoint{zone.width() / 2, zone.topLeft().y()});

    float gain_val_db = get_db(m_gain);
    float trim_val_db = get_db(m_trim);

    QString gain_text = QString::asprintf("GAIN\n%.1f dB", gain_val_db);
    QString trim_text = QString::asprintf("TRIM\n%.1f dB", trim_val_db);

    painter->drawText(gain_rect, Qt::AlignCenter, gain_text);
    painter->drawText(trim_rect, Qt::AlignCenter, trim_text);

    draw_frame(painter, zone);
}

float PipeElemAudioIn::get_db(float lin_val) {
    return 20.0f * log10(lin_val);
}

