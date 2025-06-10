#pragma once
#include <math.h>
#include <stdint.h>
#include "biquad.h"
#include "../platform/circular_buffer.h"

typedef struct {
    circular_buffer_t buffer;
    biquad_t         filter;
    float            feedback_gain;
    float            read_phase;      // Current read position
    float            current_delay_samples; // Current delay in samples
    float            target_delay_samples;  // Target delay in samples
    float            delay_slew_rate;      // How fast delay changes
    float            sample_rate;
    float delay_time_s;
} comb_t;

void comb_init(comb_t *c, size_t sample_rate) {
    circular_buffer_init(&c->buffer, (size_t)(0.2f * sample_rate));
    biquad_init(&c->filter, FILTER_LOWPASS, 20000, 0.707f, 2, sample_rate);
    c->feedback_gain = 0.0f;
    c->read_phase = 0.0f;
    c->current_delay_samples = 0.0f;
    c->target_delay_samples = 0.0f;
    c->delay_slew_rate = 0.001f; // Adjust this for smoother/faster transitions
    c->sample_rate = sample_rate;
}

float comb_process(comb_t *c, float input) {
    // Smoothly update delay time
    float delay_delta = c->target_delay_samples - c->current_delay_samples;
    c->current_delay_samples += delay_delta * c->delay_slew_rate;
    
    // Calculate read position
    float read_pos = c->buffer.write_pos - c->current_delay_samples;
    while(read_pos < 0) {
        read_pos += c->buffer.size;
    }
    
    // Interpolated read
    int read_pos_int = (int)read_pos;
    float frac = read_pos - read_pos_int;
    
    int next_pos = (read_pos_int + 1) % c->buffer.size;
    
    float sample1 = circular_buffer_read(&c->buffer, read_pos_int);
    float sample2 = circular_buffer_read(&c->buffer, next_pos);
    
    float delayed = sample1 + frac * (sample2 - sample1);
    
    // Apply filter and feedback
    float filtered = biquad_process(&c->filter, delayed);
    circular_buffer_write(&c->buffer, input + (filtered * c->feedback_gain));
    
    return delayed;
}

void comb_set_delay(comb_t *c, float delay_time, float sample_rate) {
    float new_delay = delay_time * c->sample_rate;
    
    // Bound checking
    if(new_delay >= c->buffer.size)
        new_delay = c->buffer.size - 1;
    if(new_delay < 0)
        new_delay = 0;
        
    c->target_delay_samples = new_delay;

}

void comb_set_feedback(comb_t *c, float feedback) {
    c->feedback_gain = feedback;
}

void comb_clear(comb_t *c) {
    for (size_t i = 0; i < c->buffer.size; i++) {
        circular_buffer_write(&c->buffer, 0.0f);
    }
}

float comb_get_delay_time(comb_t *c) {
    return c->current_delay_samples / c->sample_rate;
}

void comb_set_damping(comb_t *c, float damping) {
    biquad_set_frequency(&c->filter, 8000.0f * damping);
}

void comb_set_damping_frequency(comb_t *c, float freq) {
    biquad_set_frequency(&c->filter, freq);
}