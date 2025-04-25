#ifndef PIPEDESC_H
#define PIPEDESC_H

#include <optional>
#include <iostream>

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QWidget>

enum PipeElemType {
    PET_FILTER,
    PET_COMBINED_FILTER,
    PET_EFFECT,
    PET_OTHER
};

class PipeElemDesc : public QWidget {
public:
    PipeElemDesc(QWidget* parent = nullptr);
    ~PipeElemDesc() override = default;

    virtual void render_elem(QRect zone, QPainter* painter) = 0;
    void paintEvent(QPaintEvent *event) override;
};

struct PipeDesc {
    PipeElemType type;
    PipeElemDesc* desc_content;

    std::optional<PipeDesc*> next_pipe_elem;
};

#endif //PIPEDESC_H
