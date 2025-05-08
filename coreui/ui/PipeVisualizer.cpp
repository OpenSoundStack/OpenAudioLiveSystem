#include "PipeVisualizer.h"
#include "ui_PipeVisualizer.h"


PipeVisualizer::PipeVisualizer(QWidget *parent) :
    QWidget(parent), ui(new Ui::PipeVisualizer) {
    ui->setupUi(this);

    m_desc = nullptr;
}

PipeVisualizer::~PipeVisualizer() {
    delete ui;
}

void PipeVisualizer::set_pipe_content(PipeDesc *desc) {
    if (m_desc != nullptr) {
        clear_current();
    }

    PipeDesc* current_desc = desc;
    while (current_desc != nullptr) {
        current_desc->desc_content->setParent(this);

        auto* container_layout = (QVBoxLayout*)ui->elem_container->layout();
        container_layout->insertWidget(0, current_desc->desc_content);

        if (current_desc->next_pipe_elem.has_value()) {
            current_desc = current_desc->next_pipe_elem.value();
        } else {
            current_desc = nullptr;
        }
    }

    m_desc = desc;
}

void PipeVisualizer::clear_current() {
    PipeDesc* current_desc = m_desc;
    while (current_desc != nullptr) {
        ui->elem_container->layout()->removeWidget(current_desc->desc_content);

        current_desc = current_desc->next_pipe_elem.value();
    }
}

