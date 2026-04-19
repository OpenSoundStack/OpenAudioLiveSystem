// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

//
// Created by mathis on 17/04/2026.
//

// You may need to build the project (run Qt uic code generator) to get "ui_CompControl.h" resolved

#include "CompControl.h"
#include "ui_CompControl.h"

CompControl::CompControl(QWidget *parent) :
    QWidget(parent), ui(new Ui::CompControl) {
    ui->setupUi(this);

    connect(ui->dial_thresh, &QDial::valueChanged, this, [this](int value) {
        float remapped = normalize_dial(value, ui->dial_thresh);
        ui->sp_thresh->setValue(-1.0f * remapped * CompConfig::comp_depth_db);

        emit_params();
    });

    connect(ui->dial_ratio, &QDial::valueChanged, this, [this](int value) {
        if (value != ui->dial_ratio->maximum()) {
            float remapped = normalize_dial(value, ui->dial_ratio);
            ui->sp_ratio->setValue(remapped * 10.0f + 1.0f);
        } else {
            ui->sp_ratio->setValue(0);
            ui->sp_ratio->setSpecialValueText("infty:1");
        }

        emit_params();
    });

    connect(ui->dial_gain, &QDial::valueChanged, this, [this](int value) {
        float remapped = normalize_dial(value, ui->dial_gain);
        ui->sp_gain->setValue(remapped * 10.0f);

        emit_params();
    });

    connect(ui->dial_attack, &QDial::valueChanged, this, [this](int value) {
        float remapped = normalize_dial(value, ui->dial_attack);
        ui->sp_attack->setValue((int)(remapped * 120.0f));

        emit_time_params();
    });

    connect(ui->dial_release, &QDial::valueChanged, this, [this](int value) {
        float remapped = normalize_dial(value, ui->dial_release);
        ui->sp_release->setValue((int)(remapped * 500.0f));

        emit_time_params();
    });

    connect(ui->dial_hold, &QDial::valueChanged, this, [this](int value) {
        float remapped = normalize_dial(value, ui->dial_hold);
        ui->sp_hold->setValue((int)(remapped * 500.0f));

        emit_time_params();
    });

    load_defaults();
}

CompControl::~CompControl() {
    delete ui;
}

float CompControl::normalize_dial(int value, QDial *dial) {
    return (float)value / (float)dial->maximum();
}

void CompControl::map_to_dial(double value, QDial *dial, float min, float max) {
    double normalized = (value - min) / (max - min);
    double new_value = normalized * (dial->maximum() - dial->minimum()) + dial->minimum();

    dial->setValue(static_cast<int>(new_value));
}

void CompControl::emit_params() {
    emit param_changed(ui->sp_thresh->value(), ui->sp_ratio->value(), ui->sp_gain->value());
}

void CompControl::emit_time_params() {
    emit time_params_changed(ui->sp_attack->value(), ui->sp_release->value(), ui->sp_hold->value());
}

void CompControl::set_threshold(float thresh) {
    map_to_dial(-thresh, ui->dial_thresh, 0, CompConfig::comp_depth_db);
}

void CompControl::set_ratio(float ratio) {
    if (ratio == 0) {
         ui->dial_ratio->setValue(ui->dial_ratio->maximum());
    } else {
        map_to_dial(ratio, ui->dial_ratio, 1, 10);
    }
}

void CompControl::set_gain(float gain) {
    map_to_dial(gain, ui->dial_gain, 0.0f, 10.0f);
}

void CompControl::load_defaults() {
    map_to_dial(CompDefaultParams::static_defaults.threshold, ui->dial_thresh, 0, 40);
    map_to_dial(CompDefaultParams::static_defaults.ratio, ui->dial_ratio, 1, 10);
    map_to_dial(10.0f * std::log10(CompDefaultParams::static_defaults.gain), ui->dial_gain, 0, 10);

    map_to_dial(CompDefaultParams::dyn_defaults.attack_ms, ui->dial_attack, 1, 150);
    map_to_dial(CompDefaultParams::dyn_defaults.hold_ms, ui->dial_hold, 1, 500);
    map_to_dial(CompDefaultParams::dyn_defaults.release_ms, ui->dial_release, 1, 500);
}
