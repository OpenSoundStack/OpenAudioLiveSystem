#include "AudioRouterQt.h"

namespace QtWrapper {
    AudioRouterQt::AudioRouterQt(uint16_t uid) :
        QObject(nullptr), AudioRouter(uid)
    {
        m_polling_thread = QThread::create([this]() {
            while (true) {
                poll_control_packets(false);
            }
        });
    }

    bool AudioRouterQt::init_audio_router(std::string eth_interface, std::shared_ptr<NetworkMapper> nmapper) {
        if (init_router(eth_interface, nmapper)) {
            set_control_callback([this](ControlPacket& pck, LowLatHeader& hdr) {
                emit control_received(pck, hdr);
            });

            set_control_response_callback([this](ControlResponsePacket& pck, LowLatHeader& hdr) {
                emit control_response_received(pck, hdr);
            });

            m_polling_thread->start();

            return true;
        } else {
            return false;
        }
    }

} // QtWrapper