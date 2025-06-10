#pragma once

#include <stdlib.h>
#include "pitch_shift.h"
#include "harmony.h"

typedef struct {
    pitch_shift_t voices[3];
    harmony_t harmony;
    float sample_rate;
} harmonizer_t;

void harmonizer_init(harmonizer_t* h, float sample_rate) {
    h->sample_rate = sample_rate;
    for (int i = 0; i < 3; i++) {
        pitchshift_init(&h->voices[i], 6000, sample_rate);
    }
}

void harmonizer_set_harmony(harmonizer_t* h, harmony_t harmony) {
    h->harmony = harmony;
    for (int i = 0; i < h->harmony.num_voices; i++) {
        pitchshift_set_semitones(&h->voices[i], h->harmony.voices[i]);
    }
}

void harmonizer_modulate(harmonizer_t* h, float delta) {
    for (int i = 0; i < 3; i++) {
        float semitones = h->harmony.voices[i] + delta;
        pitchshift_set_semitones(&h->voices[i], semitones);
    }
}

float harmonizer_process(harmonizer_t* h, float input) {
    float output = 0;
    for (int i = 0; i < h->harmony.num_voices; i++) {
        output += pitchshift_process(&h->voices[i], input);
    }
    return output;
}