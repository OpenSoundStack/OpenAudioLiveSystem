#ifndef PIPEDESC_H
#define PIPEDESC_H

#include <optional>
#include <iostream>

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QWidget>

class PipeElemDesc : public QWidget {

    Q_OBJECT

public:
    PipeElemDesc(QWidget* parent = nullptr);
    ~PipeElemDesc() override = default;

    virtual void render_elem(QRect zone, QPainter* painter) = 0;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    QWidget* get_controllable_widget();

signals:
    void elem_selected();

private:
    bool m_being_clicked;
    bool m_selected;

protected:
    void draw_background(QPainter* painter, QRect zone);
    void draw_frame(QPainter* painter, QRect zone);

    QWidget* m_controls;
};

struct PipeDesc {
    ~PipeDesc();

    PipeElemDesc* desc_content;
    std::optional<PipeDesc*> next_pipe_elem;
};

#endif //PIPEDESC_H
