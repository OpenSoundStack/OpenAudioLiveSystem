#include "VizUtils.h"

float map_to_log_scale(float x, float smin, float smax) {
    return log10(x / smin) / log10(smax / smin);
}

float freq_to_log_scale(float f) {
    //assert(IS_BETWEEN(20, f, 20000));

    for (int step = 1; step < 5; step++) {
        float min = 1.0f * pow(10, step);
        float max = 10.0f * pow(10, step);

        if (min == 10.0f) {
            min = 20.0f;
        }

        if (IS_BETWEEN(min, f, max)) {
            // Return the freq x position in the range [0; 1]
            float mapped = (map_to_log_scale(f, min, max) + step - 1) / 4;
            return mapped * 1.21; // Cut to 20k compensation
        }
    }

    // Should not be reached. If f > 20k return 0
    return 0.0f;
}

float log_scale_to_freq(float logval) {
    float original_log_val = logval / 1.21f;
    original_log_val *= 4;

    float smin = 0.0f;
    float smax = 0.0f;
    int step = 1;

    if (IS_BETWEEN_NONINC(0.0f, original_log_val, 1.0f)) {
        smin = 20.0f;
        smax = 100.0f;
        step = 1;
    } else if (IS_BETWEEN_NONINC(1.0f, original_log_val, 2.0f)) {
        smin = 100.0f;
        smax = 1000.0f;
        step = 2;
    } else if (IS_BETWEEN_NONINC(2.0f, original_log_val, 3.0f)) {
        smin = 1000.0f;
        smax = 10000.0f;
        step = 3;
    } else if (IS_BETWEEN_NONINC(3.0f, original_log_val, 4.0f)) {
        smin = 10000.0f;
        smax = 100000.0f;
        step = 4;
    }

    original_log_val += 1;
    original_log_val -= step;
    original_log_val *= log10(smax / smin);
    float found_freq = pow(10, original_log_val) * smin;

    return found_freq;
}
