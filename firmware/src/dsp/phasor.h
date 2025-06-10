#pragma once
#include <math.h>

typedef struct {
    float increment;
    float phase;
    float sample_rate;
} phasor_t;

void phasor_init(phasor_t *p, float sample_rate) {
    p->increment = 0.0f;
    p->phase = 0.0f;
    p->sample_rate = sample_rate;
}

float phasor_get_phase(phasor_t *p) {
    return p->phase;
}

void phasor_set_phase(phasor_t *p, float phase) {
    p->phase = phase;
}

void phasor_set_frequency(phasor_t *p, float frequency) {
    p->increment = frequency / p->sample_rate;
}

float phasor_process(phasor_t *p) {
    p->phase += p->increment;
    p->phase = p->phase - floorf(p->phase);
    return p->phase;
}