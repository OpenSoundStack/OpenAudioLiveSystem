#include "SetupWindow.h"
#include "ui_SetupWindow.h"


SetupWindow::SetupWindow(QWidget *parent) :
    QWidget(parent), ui(new Ui::SetupWindow) {
    ui->setupUi(this);

    connect(ui->btn_system_view, &QPushButton::clicked, this, [this]() {
        ui->window_pages->setCurrentIndex(1); // Index 1 = system view
    });

    connect(ui->btn_sysview_back, &QPushButton::clicked, this, [this]() {
        ui->window_pages->setCurrentIndex(0);
    });
}

SetupWindow::~SetupWindow() {
    delete ui;
}
