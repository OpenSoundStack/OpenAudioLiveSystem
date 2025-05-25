#ifndef PIPEELEMNOEDIT_H
#define PIPEELEMNOEDIT_H

#include "coreui/core/PipeDesc.h"

class PipeElemNoEdit : public PipeElemDesc {
public:
    PipeElemNoEdit(QString block_name);
    ~PipeElemNoEdit() override = default;

    void render_elem(QRect zone, QPainter *painter) override;

private:
    QString m_block_name;
};



#endif //PIPEELEMNOEDIT_H
