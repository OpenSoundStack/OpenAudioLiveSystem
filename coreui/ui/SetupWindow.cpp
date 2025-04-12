#include "SetupWindow.h"
#include "ui_SetupWindow.h"


SetupWindow::SetupWindow(QWidget *parent) :
    QWidget(parent), ui(new Ui::SetupWindow) {
    ui->setupUi(this);
}

SetupWindow::~SetupWindow() {
    delete ui;
}
