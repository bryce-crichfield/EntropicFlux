#pragma once

#include <stdlib.h>
#include <math.h>
#include "delay.h"
#include "phasor.h"

typedef struct
{
    int      buffer_size;
    delay_t  d0;
    delay_t  d1;
    phasor_t p0;
    phasor_t p1;
    float    sample_rate;
} pitch_shift_t;

void pitchshift_init(pitch_shift_t* ps, int buffer_size, float sample_rate)
{
    ps->buffer_size = buffer_size;
    ps->sample_rate = sample_rate;
    delay_init(&ps->d0, buffer_size);
    delay_init(&ps->d1, buffer_size);
    phasor_init(&ps->p0, sample_rate);
    phasor_init(&ps->p1, sample_rate);
    phasor_set_phase(&ps->p0, 0.0f);
    phasor_set_phase(&ps->p1, 2 * M_PI);
}

float pitchshift_process(pitch_shift_t* ps, float input)
{
    float fade0 = phasor_process(&ps->p0);
    float fade1 = phasor_process(&ps->p1);

    float gain0 = sinf(fade0 * M_PI);
    float gain1 = sinf(fade1 * M_PI);

    float pos0 = fade0 * ps->buffer_size;
    float pos1 = fade1 * ps->buffer_size;

    delay_set_delay_samples(&ps->d0, pos0);
    delay_set_delay_samples(&ps->d1, pos1);

    delay_write(&ps->d0, input, 0);
    delay_write(&ps->d1, input, 0);

    float out0 = delay_read(&ps->d0);
    float out1 = delay_read(&ps->d1);

    return out0 * gain0 + out1 * gain1;
}

void pitchshift_set_semitones(pitch_shift_t* ps, float semitones)
{
    semitones   = -semitones;
    float ratio = powf(2.0f, -semitones / 12.0f);
    float freq  = (1.0f - ratio) * ps->sample_rate / ps->buffer_size;
    phasor_set_frequency(&ps->p0, freq);
    phasor_set_frequency(&ps->p1, freq);
}