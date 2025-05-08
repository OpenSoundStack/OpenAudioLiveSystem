#include "VizUtils.h"

float map_to_log_scale(float x, float smin, float smax) {
    return log(x / smin) / log(smax / smin);
}

float freq_to_log_scale(float f) {
    assert(IS_BETWEEN(20, f, 20000));

    for (int step = 1; step < 5; step++) {
        float min = 1.0f * pow(10, step);
        float max = 10.0f * pow(10, step);

        if (IS_BETWEEN(min, f, max)) {
            // Return the freq x position in the range [0; 1]
            float mapped = (map_to_log_scale(f, min, max) + step - 1) / 4;
            return mapped;
        }
    }

    // Should not be reached. If f > 20k return 0
    return 0.0f;
}
