// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

#ifndef OALIVESYSTEM_CORECOMPELEM_H
#define OALIVESYSTEM_CORECOMPELEM_H

#include "plugins/loader/PipeDesc.h"

class CoreCompElem : public PipeElemDesc {
public:
    CoreCompElem(AudioRouter* router);
    ~CoreCompElem() = default;

    void render_elem(QRect zone, QPainter *painter) override;
};



#endif //OALIVESYSTEM_CORECOMPELEM_H
