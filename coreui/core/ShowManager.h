#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include "coreui/ui/PipeVisualizer.h"
#include "coreui/ui/SignalWindow.h"

class ShowManager {
public:
    ShowManager();
    ~ShowManager();

    void add_pipe();
    void update_page(SignalWindow* swin);
private:
    QList<PipeVisualizer*> m_ui_show_content;
};



#endif //SHOWMANAGER_H
