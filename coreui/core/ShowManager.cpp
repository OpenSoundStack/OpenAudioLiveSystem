#include "ShowManager.h"

ShowManager::ShowManager() {

}

ShowManager::~ShowManager() {

}

void ShowManager::add_pipe() {
    m_ui_show_content.append(new PipeVisualizer{
        (int)m_ui_show_content.size()
    });
}

void ShowManager::update_page(SignalWindow *swin) {
    swin->set_page_content(m_ui_show_content);
}

