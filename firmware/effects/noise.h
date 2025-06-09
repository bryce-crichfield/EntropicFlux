#pragma once
#include <math.h>
#include <stdint.h>

typedef struct {
    float phase;
    float inc;
    float sample_rate;
    float current_value;
    float step_size;
    uint32_t rng_state;
} noise_t;

// Better initialization constant using prime numbers
#define NOISE_INIT_PRIME_1 2166136261u
#define NOISE_INIT_PRIME_2 16777619u

static inline uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static inline float uint32_to_float(uint32_t x) {
    return (float)(x >> 8) / (float)(1 << 24) * 2.0f - 1.0f;
}

void noise_init(noise_t *noise, float sample_rate) {
    // Get unique address-based seed for this instance
    uintptr_t ptr = (uintptr_t)noise;
    uint32_t instance_seed = (uint32_t)(ptr * NOISE_INIT_PRIME_1);
    
    noise->phase = 0.0f;
    noise->inc = 0.0f;
    noise->sample_rate = sample_rate;
    noise->current_value = 0.0f;
    noise->step_size = 0.01f;
    
    // Initialize RNG state using FNV-1a hash of the instance seed
    noise->rng_state = NOISE_INIT_PRIME_1;
    noise->rng_state ^= instance_seed;
    noise->rng_state *= NOISE_INIT_PRIME_2;
    
    // Warm up the RNG
    for(int i = 0; i < 10; i++) {
        xorshift32(&noise->rng_state);
    }
}

void noise_set_frequency(noise_t *noise, float freq) {
    noise->inc = freq / noise->sample_rate;
}

void noise_set_step_size(noise_t *noise, float size) {
    noise->step_size = size;
}

float noise_process(noise_t *noise) {
    noise->phase += noise->inc;
    
    if(noise->phase >= 1.0f) {
        noise->phase -= 1.0f;
        float random = uint32_to_float(xorshift32(&noise->rng_state));
        noise->current_value += random * noise->step_size;
        noise->current_value = fmaxf(-1.0f, fminf(1.0f, noise->current_value));
    }
    
    return noise->current_value;
}