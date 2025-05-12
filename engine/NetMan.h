#ifndef NETMAN_H
#define NETMAN_H

#include "OpenAudioNetwork/common/AudioPipe.h"
#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

#include <memory>


class NetMan {
public:
    NetMan();
    ~NetMan();

    void init_netman();

private:
    std::unique_ptr<NetworkMapper> m_nmapper;
    PeerConf m_pconf;
};



#endif //NETMAN_H
