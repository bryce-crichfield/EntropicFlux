#pragma once

#include <math.h>
#include "../platform/circular_buffer.h"

typedef struct {
    circular_buffer_t buffer;
    float gain;
    float read_phase;
    float current_delay_samples;
    float target_delay_samples;
    float delay_slew_rate;
    float sample_rate;
} allpass_t;

void allpass_init(allpass_t *ap, size_t sample_rate) {
    circular_buffer_init(&ap->buffer, sample_rate);
    ap->gain = 0.0f;
    ap->read_phase = 0.0f;
    ap->current_delay_samples = 0.1f * sample_rate;
    ap->target_delay_samples = ap->current_delay_samples;
    ap->delay_slew_rate = 0.001f;
    ap->sample_rate = sample_rate;
}

float allpass_process(allpass_t *ap, float input) {
    float delay_delta = ap->target_delay_samples - ap->current_delay_samples;
    ap->current_delay_samples += delay_delta * ap->delay_slew_rate;
    
    float read_pos = ap->buffer.write_pos - ap->current_delay_samples;
    while(read_pos < 0) {
        read_pos += ap->buffer.size;
    }
    
    int read_pos_int = (int)read_pos;
    float frac = read_pos - read_pos_int;
    
    int next_pos = (read_pos_int + 1) % ap->buffer.size;
    
    float sample1 = circular_buffer_read(&ap->buffer, read_pos_int);
    float sample2 = circular_buffer_read(&ap->buffer, next_pos);
    
    float delayed = sample1 + frac * (sample2 - sample1);
    float output = delayed - (ap->gain * input);
    circular_buffer_write(&ap->buffer, input + (ap->gain * delayed));
    
    return output;
}

void allpass_set_delay(allpass_t *ap, float delay_time, float sample_rate) {
    float new_delay = delay_time * ap->sample_rate;
    
    // Bound checking
    if(new_delay >= ap->buffer.size)
        new_delay = ap->buffer.size - 1;
    if(new_delay < 0)
        new_delay = 0;
        
    ap->target_delay_samples = new_delay;

}

void allpass_set_feedback(allpass_t *ap, float feedback) {
    ap->gain = feedback;
}
