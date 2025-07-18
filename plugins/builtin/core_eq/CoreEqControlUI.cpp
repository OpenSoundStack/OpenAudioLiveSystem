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

#include "CoreEqControlUI.h"

CoreEqControlUI::CoreEqControlUI() : m_filter(1000.0f, 2.0f, 2.0f, 96000.0f) {
    set_cutoff(1000.0f);
}

void CoreEqControlUI::set_cutoff(float fc) {
    m_filter.set_cutoff(fc);
    calc_filter_mag();


}

void CoreEqControlUI::calc_filter_mag() {
    m_filter_mag.clear();

    for (int i = 0; i < 20000; i++) {
        float mag = m_filter.get_filter().freq_response_magnitude(i / 96000.0f);
        m_filter_mag.emplace_back(i, mag);
    }
}

