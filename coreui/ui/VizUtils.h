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

#ifndef VIZUTILS_H
#define VIZUTILS_H

#include <cmath>
#include <assert.h>

#define IS_BETWEEN(a, x, b) ((x >= a) && (x <= b))
#define IS_BETWEEN_NONINC(a, x, b) ((x >= a) && (x < b))

float map_to_log_scale(float x, float smin, float smax);
float freq_to_log_scale(float f);
float log_scale_to_freq(float logval);

#endif //VIZUTILS_H
