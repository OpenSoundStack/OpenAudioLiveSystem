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

#ifndef COMMON_H
#define COMMON_H

// Default parameters for EQ-points
constexpr float default_frequencies[6] = {50.0f, 100.0f, 300.0f, 1000.0f, 3000.0f, 10000.0f};
constexpr float default_Q = 2.5f;

// Style variables
constexpr uint32_t handle_colors[6] = {
    0xCF1F1F,
    0xFF7708,
    0xFFF308,
    0x05FFC5,
    0x9305FF,
    0xFF05DE
};

struct FilterParams {
    float fc;
    float gain;
    float Q;
};

#endif //COMMON_H
