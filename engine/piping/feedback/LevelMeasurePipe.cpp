#include "LevelMeasurePipe.h"

#include <cmath>

LevelMeasurePipe::LevelMeasurePipe(AudioRouter* router) {
    m_sum = 0.0f;
    m_router = router;
}


float LevelMeasurePipe::process_sample(float sample) {
    static int value_counter = 0;
    constexpr float max_level = (float)(1 << 24);

    m_sum += abs(sample);

    value_counter++;
    if (value_counter > 960) {
        float mean = m_sum / value_counter;
        float mean_db = 20 * std::log10(mean / max_level);

        feedback_send(mean_db);

        value_counter = 0;
        m_sum = 0.0f;
    }

    return sample;
}

void LevelMeasurePipe::feedback_send(float db_level) {
    ControlPacket pck{};
    pck.header.type = PacketType::CONTROL;
    pck.packet_data.channel = 0;
    pck.packet_data.control_id = 0;
    pck.packet_data.control_type = DataTypes::FLOAT;
    memcpy(pck.packet_data.data, &db_level, sizeof(float));

    m_router->send_control_packet(pck, 200);
}
