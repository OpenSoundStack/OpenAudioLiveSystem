#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "coreui/ui/PipeVisualizer.h"
#include "coreui/ui/SignalWindow.h"

#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

#include "PipeElemAudioIn.h"
#include "PipeElemHPF.h"
#include "PipeElemLPF.h"

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

    void add_pipe(PipeDesc* pipe_desc);
    void update_page(SignalWindow* swin);

    void load_pipe_config();
    void load_console_config();

    void load_builtin_pipe_types();

    std::vector<std::string> get_pipe_templates();
    std::optional<std::vector<std::string>> get_template_components(const std::string& name);

    std::optional<PipeDesc*> construct_pipeline_desc(std::vector<std::string> pipeline);
    std::optional<PipeElemDesc*> construct_pipe_elem_desc(std::string pipe_type);
    void register_pipe_desc_type(std::string type, std::function<PipeElemDesc*()> callback);
private:
    QList<PipeVisualizer*> m_ui_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
    std::unordered_map<std::string, std::vector<std::string>> m_pipe_templates;

    std::unordered_map<std::string, std::function<PipeElemDesc*()>> m_pipe_desc_builder;

    NetworkConfig m_netconfig;
};



#endif //SHOWMANAGER_H
