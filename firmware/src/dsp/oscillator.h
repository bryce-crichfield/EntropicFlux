#pragma once

#include <math.h>
#include <stdlib.h>

typedef enum
{
    SINE,
    TRIANGLE,
    SAW,
    SQUARE,
    NOISE,
} oscillator_waveform_t;

typedef struct
{
    float                 phase;
    float                 inc;
    float                 sample_rate;
    oscillator_waveform_t waveform;
} oscillator_t;

void oscillator_init(oscillator_t *osc, float sample_rate)
{
    osc->phase       = 0.0f;
    osc->inc         = 0.0f;
    osc->sample_rate = sample_rate;
    osc->waveform    = SINE;
}

void oscillator_set_waveform(oscillator_t *osc, oscillator_waveform_t waveform)
{
    osc->waveform = waveform;
}

void oscillator_set_frequency(oscillator_t *osc, float freq)
{
    osc->inc = freq / osc->sample_rate;
}

float oscillator_process(oscillator_t *osc)
{
    float output = 0.0f;

    switch(osc->waveform)
    {
        case SINE: output = sinf(2.0f * M_PI * osc->phase); break;

        case TRIANGLE:
            output = 2.0f * fabsf(2.0f * osc->phase - 1.0f) - 1.0f;
            break;

        case SAW: output = 2.0f * osc->phase - 1.0f; break;

        case SQUARE: output = (osc->phase < 0.5f) ? 1.0f : -1.0f; break;

        case NOISE: output = 2.0f * ((float)rand() / RAND_MAX) - 1.0f; break;
    }

    osc->phase += osc->inc;
    if(osc->phase >= 1.0f)
    {
        osc->phase -= 1.0f;
    }

    return output;
}
