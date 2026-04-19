// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef PIPEVISUALIZER_H
#define PIPEVISUALIZER_H

#include <QWidget>

#include "../../plugins/loader/PipeDesc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PipeVisualizer; }
QT_END_NAMESPACE

class PipeVisualizer : public QWidget {
Q_OBJECT

public:
    explicit PipeVisualizer(QString pipe_name, uint16_t pid, bool unsynced = false, uint8_t channel = 0, QWidget *parent = nullptr);
    ~PipeVisualizer() override;

    void set_pipe_content(PipeDesc* desc);
    void set_pipe_name(QString name);
    void set_current_level(float db_level);

    uint8_t get_channel() const;
    uint16_t get_host() const;
    QString get_name() const;
    uint16_t get_pid() const;

    PipeDesc* get_pipe_desc();

    void control_to_elem(const ControlPacket& pck);

    void mark_synced();
signals:
    void elem_selected(PipeDesc* elem, QString pipe_name);

private:
    void clear_current();
    void sync_visual_update();

    Ui::PipeVisualizer *ui;
    PipeDesc* m_desc;

    QString m_name;
    uint8_t m_channel;

    uint16_t m_pid;

    bool m_sync_state;
};


#endif //PIPEVISUALIZER_H
