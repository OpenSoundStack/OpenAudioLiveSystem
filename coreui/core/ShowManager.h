#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "coreui/ui/PipeVisualizer.h"
#include "coreui/ui/SignalWindow.h"

#include "OpenAudioNetwork/common/NetworkMapper.h"
#include "OpenAudioNetwork/peer/peer_conf.h"

#include "PipeElemAudioIn.h"
#include "PipeElemHPF.h"
#include "PipeElemLPF.h"
#include "AudioRouterQt.h"
#include "NetworkConfig.h"
#include "DSPManager.h"

#include <qfile.h>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qqueue.h>

#include <unordered_map>

class ShowManager : public QObject {

    Q_OBJECT

public:
    ShowManager();
    ~ShowManager() override;

    bool init_console(SignalWindow* sw);

    void add_pipe(PipeDesc* pipe_desc, QString pipe_name);
    void update_page(SignalWindow* swin);

    void load_pipe_config();
    void load_console_config();

    void load_builtin_pipe_types();

    DSPManager* get_dsp_manager();

    void new_show(SignalWindow* sw);

signals:
    void elem_control_selected(QWidget* widget, QString pipe_name);

private:
    QList<PipeVisualizer*> m_ui_show_content;

    std::shared_ptr<NetworkMapper> m_nmapper;
    NetworkConfig m_netconfig;

    DSPManager* m_dsp_manager;
};



#endif //SHOWMANAGER_H
