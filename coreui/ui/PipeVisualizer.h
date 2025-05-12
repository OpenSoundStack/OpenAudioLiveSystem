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
    explicit PipeVisualizer(int pipe_number, QWidget *parent = nullptr);
    ~PipeVisualizer() override;

    void set_pipe_content(PipeDesc* desc);

private:
    void clear_current();

    Ui::PipeVisualizer *ui;
    PipeDesc* m_desc;
};


#endif //PIPEVISUALIZER_H
