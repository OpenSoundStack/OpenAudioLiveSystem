#ifndef SIGNALWINDOW_H
#define SIGNALWINDOW_H

#include <QWidget>
#include <QList>

#include "PipeVisualizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SignalWindow; }
QT_END_NAMESPACE

class SignalWindow : public QWidget {
Q_OBJECT

public:
    explicit SignalWindow(QWidget *parent = nullptr);
    ~SignalWindow() override;

    void set_page_content(const QList<PipeVisualizer*>& pipes);
private:
    Ui::SignalWindow *ui;

    QList<PipeVisualizer*> m_current_page;
};


#endif //SIGNALWINDOW_H
