// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef COREEQCONTROLUI_H
#define COREEQCONTROLUI_H

#include <array>

#include "plugins/loader/ui/FilterEditBase.h"

#include "common.h"

class CoreEqControlUI : public FilterEditBase {
public:
    CoreEqControlUI();
    ~CoreEqControlUI() override = default;

    void set_cutoff(float fc, int handle_idx) override;
    void set_gain(float gain, int handle_idx) override;
    void set_Q(float Q, int handle_idx) override;
    void calc_filter_mag() override;

    void draw_approx_filter(QPainter *painter, QRect zone) override;

    std::vector<QPointF> get_eq_curve();

private:
    void init_filters();

    std::array<FilterParams, 6> m_filters;
};



#endif //COREEQCONTROLUI_H
