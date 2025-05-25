#include "GainTrimUI.h"
#include "ui_GainTrimUI.h"


GainTrimUI::GainTrimUI(QWidget *parent) :
    QWidget(parent), ui(new Ui::GainTrimUI) {
    ui->setupUi(this);

    connect(ui->pot_gain, &QDial::valueChanged, this, [this](int value) {
        ui->label_gain->setText(QString::asprintf("%.2f dB", (float)value / 10.0f));
        trigger_value_changed();
    });

    connect(ui->pot_trim, &QDial::valueChanged, this, [this](int value) {
        ui->label_trim->setText(QString::asprintf("%.2f dB", (float)value / 10.0f));
        trigger_value_changed();
    });
}

GainTrimUI::~GainTrimUI() {
    delete ui;
}

void GainTrimUI::trigger_value_changed() {
    emit values_changed(
        ui->pot_gain->value() / 10.0f,
        ui->pot_trim->value() / 10.0f
    );
}

