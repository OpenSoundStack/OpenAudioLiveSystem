// This file is part of the Open Audio Live System project, a live audio environment
// Copyright (c) 2026 - Mathis DELGADO
//
// This project is distributed under the Creative Commons CC-BY-NC-SA licence. https://creativecommons.org/licenses/by-nc-sa/4.0

#ifndef VIZUTILS_H
#define VIZUTILS_H

#include <cmath>
#include <assert.h>

#define IS_BETWEEN(a, x, b) ((x >= a) && (x <= b))
#define IS_BETWEEN_NONINC(a, x, b) ((x >= a) && (x < b))

float map_to_log_scale(float x, float smin, float smax);
float map_to_lin_scale(float logx, float smin, float smax);
float freq_to_log_scale(float f);
float log_scale_to_freq(float logval);

#endif //VIZUTILS_H
