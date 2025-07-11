// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2025 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#include "SetupWindow.h"
#include "ui_SetupWindow.h"


SetupWindow::SetupWindow(ShowManager* sm, SignalWindow* sw, QWidget *parent) :
    QWidget(parent), ui(new Ui::SetupWindow) {
    ui->setupUi(this);
    ui->window_pages->setCurrentIndex(0);

    m_sm = sm;

    setup_add_pipe_page();

    connect(sm, &ShowManager::elem_control_selected, this, [this](QWidget* selected_elem, QString selected_name) {
        ui->window_pages->setCurrentIndex(3); // Index 3 = Element control

        if (m_current_control.has_value()) {
            m_current_control.value()->setParent(nullptr);
        }

        ui->elem_control_pname->setText(selected_name);

        selected_elem->setParent(ui->elem_control_container);
        selected_elem->show();

        m_current_control = selected_elem;
    });

    connect(sm, &ShowManager::peer_change, this, [this](QString peer_name, int peer_id, bool peer_state) {
        auto* itm = new QTreeWidgetItem{};
        itm->setData(0, 0, peer_name);
        itm->setData(1, 0, peer_id);

        ui->device_tree->addTopLevelItem(itm);
    });

    connect(ui->btn_add_pipe, &QPushButton::clicked, this, [this]() {
        ui->window_pages->setCurrentIndex(2); // Index 2 = Add Pipe
        reset_pipe_wizard();
    });

    connect(ui->btn_system_view, &QPushButton::clicked, this, [this]() {
        ui->window_pages->setCurrentIndex(1); // Index 1 = system view
    });

    connect(ui->btn_sysview_back, &QPushButton::clicked, this, [this]() {
        ui->window_pages->setCurrentIndex(0); // Index 0 = Home page
    });

    connect(ui->elem_control_btn_back, &QPushButton::clicked, this, [this]() {
        ui->window_pages->setCurrentIndex(0); // Index 0 = Home page
    });

    connect(ui->btn_new_show, &QPushButton::clicked, this, [this, sw]() {
        m_sm->new_show(sw);
    });
}

SetupWindow::~SetupWindow() {
    delete ui;
}

void SetupWindow::reset_pipe_wizard() {
    ui->new_pipe_count->setValue(1);
    ui->new_pipe_template->setCurrentIndex(0);
    ui->new_pipe_name->setText("PIPE");
}

std::optional<PipeDesc *> SetupWindow::desc_from_template_combobox() {
    auto pipeline = m_sm->get_dsp_manager()->get_template_components(ui->new_pipe_template->currentText().toStdString());
    if (!pipeline.has_value()) {
        std::cerr << "Failed to fetch pipeline template elements" << std::endl;
        return {};
    }

    auto pipe_desc = m_sm->get_dsp_manager()->construct_pipeline_desc(pipeline.value());
    if (!pipe_desc.has_value()) {
        std::cerr << "Failed to construct pipeline visualizer" << std::endl;
        return {};
    }

    return pipe_desc;
}


void SetupWindow::setup_add_pipe_page() {
    m_pipe_wiard_viz = new PipeVisualizer{0};
    m_pipe_wiard_viz->set_pipe_name("PREVIEW");

    ui->pipe_cfg_layout->insertWidget(0, m_pipe_wiard_viz);

    auto pipe_templates = m_sm->get_dsp_manager()->get_pipe_templates();
    for (auto& preset : pipe_templates) {
        ui->new_pipe_template->addItem(QString::fromStdString(preset));
    }

    connect(ui->new_pipe_template, &QComboBox::currentIndexChanged, this, [this]() {
        auto pipe_desc = desc_from_template_combobox();

        if (pipe_desc.has_value()) {
            m_pipe_wiard_viz->set_pipe_content(pipe_desc.value());
            ui->new_pipe_ok->setEnabled(true);
        } else {
            QMessageBox::warning(this, "ERROR", "Failed to find pipe template.");
            ui->new_pipe_ok->setEnabled(false);
        }
    });

    connect(ui->new_pipe_ok, &QPushButton::clicked, this, [this]() {
        DSPManager* dsp_mgr = m_sm->get_dsp_manager();

        for (int i = 1; i <= ui->new_pipe_count->value(); i++) {
            auto pipe_desc = desc_from_template_combobox();
            QString pipe_name = ui->new_pipe_name->text();

            // No need to add number if only one pipe is created
            if (ui->new_pipe_count->value() > 1) {
                pipe_name +=  " %1";
                pipe_name = pipe_name.arg(i);
            }

            //m_sm->add_pipe(pipe_desc.value(), pipe_name); // Considered valid because already checked in combo box
            //m_sm->update_page(m_sw);

            // Sync to DSP
            // Optional should contain something. The check has already been done before
            auto pipeline = dsp_mgr->get_template_components(ui->new_pipe_template->currentText().toStdString());
            dsp_mgr->add_pipeline_to_sync_queue(pipeline.value(), pipe_desc.value(), pipe_name);
        }

        dsp_mgr->sync_queue_to_dsp();
    });

    connect(ui->new_pipe_name, &QLineEdit::textChanged, this, [this]() {
        QString name = ui->new_pipe_name->text();
        if (name.isEmpty()) {
            ui->new_pipe_ok->setEnabled(false);
        } else {
            ui->new_pipe_ok->setEnabled(true);
        }

        m_pipe_wiard_viz->set_pipe_name(name);
    });

    connect(ui->new_pipe_cancel, &QPushButton::clicked, this, [this]() {
        // Go back to home page
        ui->window_pages->setCurrentIndex(0);
    });

    // Update UI with preselected pipeline template
    ui->new_pipe_template->currentIndexChanged(0);
}
