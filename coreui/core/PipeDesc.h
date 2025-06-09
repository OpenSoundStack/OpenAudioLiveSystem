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

#ifndef PIPEDESC_H
#define PIPEDESC_H

#include <optional>
#include <iostream>
#include <unordered_map>
#include <memory>

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QWidget>

#include "../core/ElemControlData.h"
#include "OpenAudioNetwork/common/AudioRouter.h"

enum ElemFlags {
    ELEM_NO_FLAGS = 0,
    ELEM_IS_SIMPLE_IO = 1,
    ELEM_IS_INPUT_MATRIX = 1 << 1,
    ELEM_IS_OUTPUT_MATRIX = 1 << 2
};

class PipeElemDesc : public QWidget {

    Q_OBJECT

public:
    PipeElemDesc(AudioRouter* router, QWidget* parent = nullptr);
    ~PipeElemDesc() override = default;

    virtual void render_elem(QRect zone, QPainter* painter) = 0;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    QWidget* get_controllable_widget();

    void index_pipe(int index);
    int get_index();

    void set_channel(uint8_t channel);
    uint8_t get_channel();

    void set_host(uint16_t host);
    uint16_t get_host();

    void register_control(uint8_t control_id, std::shared_ptr<ElemControlData> control_data);
    void send_control_packets();

    ElemFlags get_flags();
signals:
    void elem_selected();

private:
    bool m_being_clicked;
    bool m_selected;

protected:
    void draw_background(QPainter* painter, QRect zone);
    void draw_frame(QPainter* painter, QRect zone);

    AudioRouter* m_router;

    QWidget* m_controls;
    std::unordered_map<int, std::shared_ptr<ElemControlData>> m_control_data;

    int m_index;
    uint8_t m_channel;
    uint16_t m_dsp_host;

    ElemFlags m_flags;
};

struct PipeDesc {
    ~PipeDesc();

    void index_pipes();
    void set_pipe_channel(uint8_t channel, uint16_t host);

    PipeElemDesc* desc_content;
    std::optional<PipeDesc*> next_pipe_elem;
};

#endif //PIPEDESC_H
