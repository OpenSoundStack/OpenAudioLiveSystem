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
signals:
    void elem_selected();

private:
    bool m_being_clicked;
    bool m_selected;

    AudioRouter* m_router;

protected:
    void draw_background(QPainter* painter, QRect zone);
    void draw_frame(QPainter* painter, QRect zone);

    QWidget* m_controls;
    std::unordered_map<int, std::shared_ptr<ElemControlData>> m_control_data;

    int m_index;
    uint8_t m_channel;
    uint16_t m_dsp_host;
};

struct PipeDesc {
    ~PipeDesc();

    void index_pipes();
    void set_pipe_channel(uint8_t channel, uint16_t host);

    PipeElemDesc* desc_content;
    std::optional<PipeDesc*> next_pipe_elem;
};

#endif //PIPEDESC_H
