#pragma once

#include <math.h>

float softclip_process(float in) {
    float x = fmaxf(-3.0f, fminf(3.0f, in));
    return 0.5f * (1.0f + x / (1.0f + fabsf(x)));
}