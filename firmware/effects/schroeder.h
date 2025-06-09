#pragma once

#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "allpass.h"
#include "comb.h"
#include "noise.h"

#define SCHROEDER_COMB_COUNT 16
#define SCHROEDER_ALLPASS_COUNT 4

static float SCHROEDER_COMB_DELAYS_S[SCHROEDER_COMB_COUNT];
static float SCHROEDER_COMB_FEEDBACKS[SCHROEDER_COMB_COUNT];

static const float SCHROEDER_ALLPASS_DELAYS_S[SCHROEDER_ALLPASS_COUNT] = {0.013f, 0.023f, 0.037f,
                                                                          0.019f};

typedef struct {
    comb_t comb[SCHROEDER_COMB_COUNT];
    noise_t comb_noise[SCHROEDER_COMB_COUNT];
    biquad_t damping_filter[SCHROEDER_COMB_COUNT];
    allpass_t allpass[SCHROEDER_ALLPASS_COUNT];
    noise_t allpass_noise[SCHROEDER_ALLPASS_COUNT];
    float mix;
    float feedback;
    float modulation_rate;
    size_t sample_rate;
    float damping_frequency;

    float damping_lfo_phase;
    float damping_lfo_rate;
    float damping_mod_depth;  // 0.0 to 1.0
} schroeder_t;

void schroeder_init(schroeder_t* schroeder, size_t sample_rate);
float schroeder_process(schroeder_t* schroeder, float input);
void schroeder_set_decay(schroeder_t* schroeder, float t60_seconds);
void schroder_set_modulation_rate(schroeder_t* schroeder, float rate);
void schroeder_set_damping(schroeder_t* schroeder, float damping);


void schroeder_init(schroeder_t* schroeder, size_t sample_rate) {
    // Initialize random comb delays
    srand(time(NULL));
    float delay_min = 0.01f;
    float delay_max = 0.15f;
    for (int i = 0; i < SCHROEDER_COMB_COUNT; i++) {
        SCHROEDER_COMB_DELAYS_S[i] =
            delay_min + ((float)rand() / RAND_MAX) * (delay_max - delay_min);
    }

    // Initialize comb filters
    for (int i = 0; i < SCHROEDER_COMB_COUNT; i++) {
        comb_init(&schroeder->comb[i], sample_rate);
        comb_set_delay(&schroeder->comb[i], SCHROEDER_COMB_DELAYS_S[i], sample_rate);
        comb_set_feedback(&schroeder->comb[i], schroeder->feedback);
        noise_init(&schroeder->comb_noise[i], sample_rate);
        noise_set_frequency(&schroeder->comb_noise[i], 15);
        noise_set_step_size(&schroeder->comb_noise[i], 0.1f);
        biquad_init(&schroeder->damping_filter[i], FILTER_LOWPASS, 20000, 0.707f, 1, sample_rate);
    }

    // Initialize allpass filters
    for (int i = 0; i < SCHROEDER_ALLPASS_COUNT; i++) {
        allpass_init(&schroeder->allpass[i], sample_rate);
        allpass_set_delay(&schroeder->allpass[i], SCHROEDER_ALLPASS_DELAYS_S[i], sample_rate);
        allpass_set_feedback(&schroeder->allpass[i], 0.5f);
        noise_init(&schroeder->allpass_noise[i], sample_rate);
        noise_set_frequency(&schroeder->allpass_noise[i], 15);
        noise_set_step_size(&schroeder->allpass_noise[i], 0.015f);
    }

    schroeder->sample_rate = sample_rate;
    schroeder->feedback = 0.85f;
    schroeder->modulation_rate = 10.0f;
    schroeder_set_decay(schroeder, 1.0f);

    schroeder->damping_lfo_phase = 0.0f;
    schroeder->damping_lfo_rate = 0.01f;
    schroeder->damping_mod_depth = 1.0f;
}

float schroeder_process(schroeder_t* schroeder, float input) {
    float output = 0.0f;

    // Sum parallel comb filters
    float comb_scale = 1.0f / sqrtf((float)SCHROEDER_COMB_COUNT);
    for (int j = 0; j < SCHROEDER_COMB_COUNT; j++) {
        // Get the noise value and map it from [-1, 1] to [0.5, 1]
        float noise_value = noise_process(&schroeder->comb_noise[j]);
        noise_value = 0.5f + (noise_value * 0.25f);

        float base_delay = SCHROEDER_COMB_DELAYS_S[j];
        float modulation_offset = (noise_value)*base_delay;

        float delay_time = base_delay + modulation_offset;

        // Add safety clamp to prevent negative delay times
        delay_time = fmaxf(delay_time, 0.001f);

        float phase_delta = (2.0f * M_PI * schroeder->damping_lfo_rate) / schroeder->sample_rate;
        schroeder->damping_lfo_phase += phase_delta;
        float mod = sinf(schroeder->damping_lfo_phase);

        // Scale modulation range based on base frequency
        // float mod_range = schroeder->damping_frequency * 0.5f;  // 50% deviation
        // float freq = schroeder->damping_frequency + (mod * mod_range);

        // Safety clamp to prevent negative frequencies
        // freq = fmaxf(20.0f, freq);

        // biquad_set_frequency(&schroeder->damping_filter[j], freq);

        comb_set_delay(&schroeder->comb[j], delay_time, schroeder->sample_rate);
        comb_t* comb = &schroeder->comb[j];
        float comb_value = comb_process(comb, input) * comb_scale;
        comb_value = biquad_process(&schroeder->damping_filter[j], comb_value);
        output += comb_value;
    }

    // Apply series allpass filters
    for (int i = 0; i < SCHROEDER_ALLPASS_COUNT; i++) {
        float noise_value = noise_process(&schroeder->allpass_noise[i]);
        noise_value = 0.5f + (noise_value * 0.25f);

        float base_delay = SCHROEDER_ALLPASS_DELAYS_S[i];
        float modulation_offset = (noise_value)*base_delay;

        float delay_time = base_delay + modulation_offset;

        // Add safety clamp to prevent negative delay times
        delay_time = fmaxf(delay_time, 0.001f);

        allpass_set_delay(&schroeder->allpass[i], delay_time, schroeder->sample_rate);
        output = allpass_process(&schroeder->allpass[i], output);
    }

    return output;
}

// Set the decay time of the reverb in seconds
void schroeder_set_decay(schroeder_t* schroeder, float t60_seconds) {
    for (int i = 0; i < SCHROEDER_COMB_COUNT; i++) {
        float delay_secs = SCHROEDER_COMB_DELAYS_S[i];
        float feedback = expf(-6.907755f * delay_secs / t60_seconds);
        comb_set_feedback(&schroeder->comb[i], feedback);
    }
}

void schroder_set_modulation_rate(schroeder_t* schroeder, float rate) {
    schroeder->modulation_rate = fmaxf(rate, 0.0f);
    rate = powf(rate, 2.0f);
    float comb_rate = 0.001f + (rate * 0.099f);
    for (int i = 0; i < SCHROEDER_COMB_COUNT; i++) {
        noise_set_step_size(&schroeder->comb_noise[i], comb_rate);
    }
    for (int i = 0; i < SCHROEDER_ALLPASS_COUNT; i++) {
        noise_set_step_size(&schroeder->allpass_noise[i], comb_rate);
    }
}

void schroeder_set_damping(schroeder_t* schroeder, float damping) {
    damping = fmaxf(0.0f, fminf(damping, 1.0f));
    damping = powf(damping, 2.0f);
    float freq = 2000.0f + (18000.0f * (1.0f - damping));
    schroeder->damping_frequency = freq;
    for (int i = 0; i < SCHROEDER_COMB_COUNT; i++) {
        biquad_set_frequency(&schroeder->damping_filter[i], freq);
        comb_set_damping_frequency(&schroeder->comb[i], freq);
    }
}