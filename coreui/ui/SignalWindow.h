#ifndef SIGNALWINDOW_H
#define SIGNALWINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class SignalWindow; }
QT_END_NAMESPACE

class SignalWindow : public QWidget {
Q_OBJECT

public:
    explicit SignalWindow(QWidget *parent = nullptr);
    ~SignalWindow() override;

private:
    Ui::SignalWindow *ui;
};


#endif //SIGNALWINDOW_H
