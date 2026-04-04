// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef FILTERVIZLPF_H
#define FILTERVIZLPF_H

#include "plugins/loader/ui/FilterEditBase.h"

class FilterVizLPF : public FilterEditBase {
public:
    FilterVizLPF();
    ~FilterVizLPF() override = default;

    void set_cutoff(float fc, int handle_idx) override;

protected:
    void draw_approx_filter(QPainter *painter, QRect zone) override;
};



#endif //FILTERVIZLPF_H
