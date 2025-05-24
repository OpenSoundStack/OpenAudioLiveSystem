#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QWidget>
#include <QMessageBox>

#include "../core/ShowManager.h"
#include "PipeVisualizer.h"
#include "SignalWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SetupWindow; }
QT_END_NAMESPACE

class SetupWindow : public QWidget {
Q_OBJECT

public:
    explicit SetupWindow(ShowManager* sm, SignalWindow* sw, QWidget *parent = nullptr);
    ~SetupWindow() override;

    void reset_pipe_wizard();
    void setup_add_pipe_page();
private:
    std::optional<PipeDesc*> desc_from_template_combobox();

    Ui::SetupWindow *ui;
    ShowManager* m_sm;

    PipeVisualizer* m_pipe_wiard_viz;
    std::optional<QWidget*> m_current_control;
};


#endif //SETUPWINDOW_H
