#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "coreui/ui/PipeVisualizer.h"
#include "coreui/ui/SignalWindow.h"

#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>

#include <unordered_map>

struct NetworkConfig {
    std::string eth_interface;
    uint16_t uid;

    QJsonObject serialize();
};

class ShowManager {
public:
    ShowManager();
    ~ShowManager();

    bool init_console();

    void add_pipe();
    void update_page(SignalWindow* swin);

    void load_pipe_config();
    void load_console_config();
private:
    QList<PipeVisualizer*> m_ui_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
    std::unordered_map<std::string, std::vector<std::string>> m_pipe_templates;

    NetworkConfig m_netconfig;
};



#endif //SHOWMANAGER_H
