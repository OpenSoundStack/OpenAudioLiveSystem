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

#ifndef FILTERVIZLPF_H
#define FILTERVIZLPF_H

#include "plugins/loader/ui/FilterEditBase.h"

class FilterVizLPF : public FilterEditBase {
public:
    FilterVizLPF();
    ~FilterVizLPF() override = default;

    void set_cutoff(float fc) override;

protected:
    void draw_approx_filter(QPainter *painter, QRect zone) override;
};



#endif //FILTERVIZLPF_H
