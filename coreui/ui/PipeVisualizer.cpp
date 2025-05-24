#include "PipeVisualizer.h"

#include <utility>
#include "ui_PipeVisualizer.h"


PipeVisualizer::PipeVisualizer(QString pipe_name, uint8_t channel, QWidget *parent) :
    QWidget(parent), ui(new Ui::PipeVisualizer) {
    ui->setupUi(this);

    m_desc = nullptr;
    m_name = std::move(pipe_name);
    m_channel = channel;

    ui->label->setText(m_name);
    set_current_level(-60.0f);
}

PipeVisualizer::~PipeVisualizer() {
    delete ui;
    delete m_desc;
}

void PipeVisualizer::set_pipe_content(PipeDesc *desc) {
    if (m_desc != nullptr) {
        clear_current();
        delete m_desc;
    }

    PipeDesc* current_desc = desc;

    int insert_index = 0;
    while (current_desc != nullptr) {
        current_desc->desc_content->setParent(this);

        auto* container_layout = (QVBoxLayout*)ui->elem_container->layout();
        container_layout->insertWidget(insert_index, current_desc->desc_content);

        // Event propagation
        connect(current_desc->desc_content, &PipeElemDesc::elem_selected, this, [this, current_desc]() {
            emit elem_selected(current_desc, m_name);
        });

        if (current_desc->next_pipe_elem.has_value()) {
            current_desc = current_desc->next_pipe_elem.value();
        } else {
            current_desc = nullptr;
        }

        insert_index++;
    }

    m_desc = desc;
}

void PipeVisualizer::clear_current() {
    PipeDesc* current_desc = m_desc;
    while (current_desc != nullptr) {
        ui->elem_container->layout()->removeWidget(current_desc->desc_content);

        if (current_desc->next_pipe_elem.has_value()) {
            current_desc = current_desc->next_pipe_elem.value();
        } else {
            current_desc = nullptr;
        }
    }
}

void PipeVisualizer::set_pipe_name(QString name) {
    ui->label->setText(name);
    m_name = name;
}

PipeDesc *PipeVisualizer::get_pipe_desc() {
    return m_desc;
}

void PipeVisualizer::set_current_level(float db_level) {
    ui->signal_level->setValue((int)(db_level * 10));
}

uint8_t PipeVisualizer::get_channel() const {
    return m_channel;
}
