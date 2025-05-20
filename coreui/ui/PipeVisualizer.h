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
    explicit PipeVisualizer(QString pipe_name, QWidget *parent = nullptr);
    ~PipeVisualizer() override;

    void set_pipe_content(PipeDesc* desc);
    void set_pipe_name(QString name);

    PipeDesc* get_pipe_desc();
private:
    void clear_current();

    Ui::PipeVisualizer *ui;
    PipeDesc* m_desc;

    QString m_name;
};


#endif //PIPEVISUALIZER_H
