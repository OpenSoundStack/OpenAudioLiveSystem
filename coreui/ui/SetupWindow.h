#ifndef SETUPWINDOW_H
#define SETUPWINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class SetupWindow; }
QT_END_NAMESPACE

class SetupWindow : public QWidget {
Q_OBJECT

public:
    explicit SetupWindow(QWidget *parent = nullptr);
    ~SetupWindow() override;

private:
    Ui::SetupWindow *ui;
};


#endif //SETUPWINDOW_H
