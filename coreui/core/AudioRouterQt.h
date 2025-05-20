#ifndef AUDIOROUTERQT_H
#define AUDIOROUTERQT_H

#include <QObject>
#include <QThread>

#include "OpenAudioNetwork/common/AudioRouter.h"
#include "OpenAudioNetwork/common/packet_structs.h"

namespace QtWrapper {

class AudioRouterQt : public QObject, AudioRouter {

    Q_OBJECT

public:
    AudioRouterQt(uint16_t uid);
    ~AudioRouterQt() override = default;

    bool init_audio_router(std::string eth_interface, std::shared_ptr<NetworkMapper> nmapper);

signals:
    void control_received(ControlPacket pck, LowLatHeader hdr);
    void control_response_received(ControlResponsePacket pck, LowLatHeader hdr);

private:
    QThread* m_polling_thread;
};

} // QtWrapper

#endif //AUDIOROUTERQT_H
