#include "LevelMeasurePipe.h"

LevelMeasurePipe::LevelMeasurePipe(AudioRouter* router, std::shared_ptr<NetworkMapper> nmapper) {
    m_router = router;
    m_nmapper = nmapper;

    m_value_counter = 0;
    m_sum = 0.0f;

    // Initialize sample list
    // 28000 samples = 300 ms @ 96 kHz sampling rate
    for (int i = 0; i < 28000; i++) {
        m_rms_buffer.push_back(0);
    }
}


float LevelMeasurePipe::process_sample(float sample) {
    // Efficient RMS calculation
    // Add the latest sample and subtract the oldest
    // Avoids to read the ENTIRE 28k element list every sample...

    float sample_sat = sample;
    if (abs(sample) > 1.0f) {
        sample_sat = 1.0f;
    }

    float sample2 = sample_sat * sample_sat;
    m_sum += sample2;

    m_rms_buffer.push_back(sample2);

    m_sum -= m_rms_buffer.front();
    m_rms_buffer.pop_front();

    m_value_counter++;
    if (m_value_counter > 300) {
        float temp_sum = 0.0f;
        for (auto& val : m_rms_buffer) {
            temp_sum += val;
        }

        float mean = m_sum / 28000.0f;
        float rms = std::sqrt(mean);
        float mean_db = 20 * std::log10(rms); // Max level is 1.0f

        feedback_send(mean_db);

        m_value_counter = 0;
    }

    // Return original sample
    return sample;
}

void LevelMeasurePipe::feedback_send(float db_level) {
    ControlPacket pck{};
    pck.header.type = PacketType::CONTROL;
    pck.packet_data.channel = get_channel();
    pck.packet_data.control_id = 0;
    pck.packet_data.control_type = DataTypes::FLOAT;
    memcpy(pck.packet_data.data, &db_level, sizeof(float));

    auto surfaces = m_nmapper->find_all_control_surfaces();

    for (auto& surface : surfaces) {
        m_router->send_control_packet(pck, surface);
    }
}
