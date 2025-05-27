#include "FilterVizHPF.h"

FilterVizHPF::FilterVizHPF() {
    FilterVizHPF::set_cutoff(100.0f);
}

void FilterVizHPF::set_cutoff(float fc) {
    m_fc = fc;
    update();
}

void FilterVizHPF::draw_approx_filter(QPainter *painter, QRect zone) {
    QPainterPath path{};
    path.moveTo(QPoint{zone.width(), zone.height() / 2});

    float freq_x_pos = freq_to_log_scale(m_fc) * zone.width();

    // Computing characteristic points for the slope to be coherent
    float freq_10k_x_pos = freq_to_log_scale(10000.0f) * zone.width();
    float freq_1k_x_pos = freq_to_log_scale(1000.0f) * zone.width();
    float decade_distance = freq_10k_x_pos - freq_1k_x_pos;

    float freq2_x_pos = freq_x_pos - decade_distance;

    float stopband_level = -36; // -36 dB
    stopband_level = (stopband_level + 18.0f) / 36.0f;
    float stopband_y = zone.height() - stopband_level * zone.height();

    path.lineTo(QPoint{(int)freq_x_pos, zone.height() / 2});
    path.lineTo(QPoint{(int)freq2_x_pos, (int)stopband_y});

    painter->drawPath(path);
}
