#pragma once

#include "pitch_shift.h"
#include "oscillator.h"

class Vibrato
{
    oscillator_t lfo;
    pitch_shift_t pitch_shift;
    size_t sample_rate;
    float depth;
public:
    Vibrato() {}

    void Init(size_t sample_rate)
    {
        this->sample_rate = sample_rate;
        oscillator_init(&lfo, sample_rate);
        pitchshift_init(&pitch_shift, 6000, sample_rate);
    }

    void SetRate(float rate)
    {
        oscillator_set_frequency(&lfo, rate);
    }

    void SetDepth(float depth)
    {
        this->depth = depth;
    }

    float Process(float input)
    {
        float lfo_val = oscillator_process(&lfo);
        pitchshift_set_semitones(&pitch_shift, lfo_val * depth);
        return pitchshift_process(&pitch_shift, input);
    }
};