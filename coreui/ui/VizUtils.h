#ifndef VIZUTILS_H
#define VIZUTILS_H

#include <cmath>
#include <assert.h>

#define IS_BETWEEN(a, x, b) ((x >= a) && (x < b))

float map_to_log_scale(float x, float smin, float smax);
float freq_to_log_scale(float f);

#endif //VIZUTILS_H
