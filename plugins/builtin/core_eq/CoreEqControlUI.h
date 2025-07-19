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

#ifndef COREEQCONTROLUI_H
#define COREEQCONTROLUI_H

#include "plugins/loader/ui/FilterEditBase.h"

#include "OpenDSP/src/filter/audio/peak.h"

class CoreEqControlUI : public FilterEditBase {
public:
    CoreEqControlUI();
    ~CoreEqControlUI() override = default;

    void set_cutoff(float fc, int handle_idx) override;
    void set_gain(float gain, int handle_idx) override;
    void calc_filter_mag() override;

    void draw_approx_filter(QPainter *painter, QRect zone) override;

private:
    PeakFilter m_filter;
};



#endif //COREEQCONTROLUI_H
