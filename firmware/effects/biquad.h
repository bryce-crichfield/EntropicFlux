#pragma once

// #include <arm_math.h>
#include <math.h>
#include <stdlib.h>

#include "../platform/fast_trig.h"

#define BIQUAD_MAX_STAGES 4

typedef enum {
    FILTER_LOWPASS,
    FILTER_HIGHPASS,
    FILTER_BANDPASS,
    FILTER_LOWSHELF,
    FILTER_HIGHSHELF
} filter_type_t;

typedef struct {
    float a0[BIQUAD_MAX_STAGES], a1[BIQUAD_MAX_STAGES], a2[BIQUAD_MAX_STAGES],
        b0[BIQUAD_MAX_STAGES], b1[BIQUAD_MAX_STAGES], b2[BIQUAD_MAX_STAGES];
    float x1[BIQUAD_MAX_STAGES], x2[BIQUAD_MAX_STAGES], y1[BIQUAD_MAX_STAGES],
        y2[BIQUAD_MAX_STAGES];
    float freq, q, sample_rate;
    filter_type_t type;
    int num_stages;  // Number of biquad stages
    float gain_db;
} biquad_t;

// Modify coefficient calculation
void biquad_calculate_coeffs(biquad_t *f) {
    for (int i = 0; i < f->num_stages; i++) {
        float omega = 2.0f * M_PI * f->freq / f->sample_rate;
        float alpha = sinf(omega) / (2.0f * f->q);
        float cos_omega = cosf(omega);
        float A = powf(10.0f, f->gain_db / 40.0f);  // Convert dB to linear gain
        float beta = sqrtf(A) / f->q;
        switch (f->type) {
            case FILTER_LOWPASS:
                f->b0[i] = (1.0f - cos_omega) / 2.0f;
                f->b1[i] = 1.0f - cos_omega;
                f->b2[i] = (1.0f - cos_omega) / 2.0f;
                break;
            case FILTER_HIGHPASS:
                f->b0[i] = (1.0f + cos_omega) / 2.0f;
                f->b1[i] = -(1.0f + cos_omega);
                f->b2[i] = (1.0f + cos_omega) / 2.0f;
                break;
            case FILTER_BANDPASS:
                f->b0[i] = alpha;
                f->b1[i] = 0.0f;
                f->b2[i] = -alpha;
                break;
            case FILTER_LOWSHELF:
                f->b0[i] = A * ((A + 1.0f) - (A - 1.0f) * cos_omega + beta * sinf(omega));
                f->b1[i] = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_omega);
                f->b2[i] = A * ((A + 1.0f) - (A - 1.0f) * cos_omega - beta * sinf(omega));
                f->a0[i] = (A + 1.0f) + (A - 1.0f) * cos_omega + beta * sinf(omega);
                f->a1[i] = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_omega);
                f->a2[i] = (A + 1.0f) + (A - 1.0f) * cos_omega - beta * sinf(omega);
                break;
            case FILTER_HIGHSHELF:
                f->b0[i] = A * ((A + 1.0f) + (A - 1.0f) * cos_omega + beta * sinf(omega));
                f->b1[i] = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_omega);
                f->b2[i] = A * ((A + 1.0f) + (A - 1.0f) * cos_omega - beta * sinf(omega));
                f->a0[i] = (A + 1.0f) - (A - 1.0f) * cos_omega + beta * sinf(omega);
                f->a1[i] = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_omega);
                f->a2[i] = (A + 1.0f) - (A - 1.0f) * cos_omega - beta * sinf(omega);
                break;
        }

        f->a0[i] = 1.0f + alpha;
        f->a1[i] = -2.0f * cos_omega;
        f->a2[i] = 1.0f - alpha;

        float norm = 1.0f / f->a0[i];
        f->b0[i] *= norm;
        f->b1[i] *= norm;
        f->b2[i] *= norm;
        f->a1[i] *= norm;
        f->a2[i] *= norm;
    }
}
void biquad_init(biquad_t *f, filter_type_t type, float freq, float q, size_t stages,
                 float sample_rate) {
    if (stages > BIQUAD_MAX_STAGES) {
        stages = BIQUAD_MAX_STAGES;
    }
    f->num_stages = stages;
    for (int i = 0; i < f->num_stages; i++) {
        f->x1[i] = f->x2[i] = f->y1[i] = f->y2[i] = 0.0f;
    }
    f->type = type;
    f->freq = freq;
    f->q = q;
    f->sample_rate = sample_rate;
    f->gain_db = 0.0f;
    biquad_calculate_coeffs(f);
}
float biquad_process(biquad_t *f, float input) {
    float temp = input;
    for (int i = 0; i < f->num_stages; i++) {
        float output = f->b0[i] * temp + f->b1[i] * f->x1[i] + f->b2[i] * f->x2[i] -
                       f->a1[i] * f->y1[i] - f->a2[i] * f->y2[i];

        f->x2[i] = f->x1[i];
        f->x1[i] = temp;
        f->y2[i] = f->y1[i];
        f->y1[i] = output;

        temp = output;
    }
    return temp;
}

void biquad_set_frequency(biquad_t *f, float freq) {
    f->freq = freq;
    biquad_calculate_coeffs(f);
}

void biquad_set_q(biquad_t *f, float q) {
    f->q = q;
    biquad_calculate_coeffs(f);
}

void biquad_modulate(biquad_t *f, float delta) {
    float set_freq = f->freq;
    f->freq += delta;
    biquad_calculate_coeffs(f);
    f->freq = set_freq;
}

void biquad_set_gain(biquad_t *f, float gain_db) {
    f->gain_db = gain_db;
    biquad_calculate_coeffs(f);
}