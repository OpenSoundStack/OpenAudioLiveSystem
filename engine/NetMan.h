#ifndef NETMAN_H
#define NETMAN_H

#include "OpenAudioNetwork/common/AudioPipe.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"
#include "OpenAudioNetwork/netutils/LowLatSocket.h"

#include "log.h"

#include <memory>


class NetMan {
public:
    NetMan();
    ~NetMan();

    bool init_netman();
    void update_netman();

private:
    std::shared_ptr<NetworkMapper> m_nmapper;
    PeerConf m_pconf;

    std::unique_ptr<LowLatSocket> m_dsp_control;
};



#endif //NETMAN_H
