#ifndef PIPEVISUALIZER_H
#define PIPEVISUALIZER_H

#include <QWidget>

#include "../core/PipeDesc.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PipeVisualizer; }
QT_END_NAMESPACE

class PipeVisualizer : public QWidget {
Q_OBJECT

public:
    explicit PipeVisualizer(QString pipe_name, uint8_t channel = 0, QWidget *parent = nullptr);
    ~PipeVisualizer() override;

    void set_pipe_content(PipeDesc* desc);
    void set_pipe_name(QString name);
    void set_current_level(float db_level);

    uint8_t get_channel() const;

    PipeDesc* get_pipe_desc();
signals:
    void elem_selected(PipeDesc* elem, QString pipe_name);

private:
    void clear_current();

    Ui::PipeVisualizer *ui;
    PipeDesc* m_desc;

    QString m_name;
    uint8_t m_channel;
};


#endif //PIPEVISUALIZER_H
