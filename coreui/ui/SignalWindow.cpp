#include "SignalWindow.h"
#include "ui_SignalWindow.h"

SignalWindow::SignalWindow(QWidget *parent) :
    QWidget(parent), ui(new Ui::SignalWindow) {
    ui->setupUi(this);
}

SignalWindow::~SignalWindow() {
    delete ui;
}
