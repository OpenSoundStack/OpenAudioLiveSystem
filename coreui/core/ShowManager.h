#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "coreui/ui/PipeVisualizer.h"
#include "coreui/ui/SignalWindow.h"

#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

class ShowManager {
public:
    ShowManager();
    ~ShowManager();

    bool init_console();

    void add_pipe();
    void update_page(SignalWindow* swin);
private:
    QList<PipeVisualizer*> m_ui_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
};



#endif //SHOWMANAGER_H
