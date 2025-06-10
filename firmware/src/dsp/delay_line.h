#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include "platform/memory.h"


typedef struct
{
    int    buffer_size;
    float* buffer;
    int    write_head;
    float  read_head;      
    float  target_read_head;
    float  smoothing;      
    size_t is_bypassed;
} delay_line_t;

void delay_line_init(delay_line_t* d, int buffer_size)
{
    d->buffer_size = buffer_size;
    d->buffer      = (float*)allocate(buffer_size * sizeof(float));
    zero_memory(d->buffer, buffer_size * sizeof(float));
    d->write_head  = 0;
    d->read_head   = 0;
    d->target_read_head = 0;
    d->smoothing   = 0.997f;
    d->is_bypassed = 0;
}

void delay_line_set_delay_line_samples(delay_line_t* d, float delay)
{
    if(delay <= 0)
    {
        d->is_bypassed = 1;
        return;
    }

    d->is_bypassed = 0;

    float new_read_pos = d->write_head - delay;
    while(new_read_pos < 0)
        new_read_pos += d->buffer_size;
    d->target_read_head = new_read_pos;
}

void delay_line_set_delay_line_time(delay_line_t* d, float delay_line_time, float sample_rate)
{
    delay_line_set_delay_line_samples(d, delay_line_time * sample_rate);
}

float delay_line_read_interpolated(delay_line_t* d)
{
    int pos1 = (int)d->read_head;
    int pos2 = (pos1 + 1) % d->buffer_size;
    float frac = d->read_head - (float)pos1;
    
    float s1 = d->buffer[pos1];
    float s2 = d->buffer[pos2];
    
    return s1 + frac * (s2 - s1);
}

void delay_line_write(delay_line_t* d, float input, float feedback)
{
    float current = delay_line_read_interpolated(d);
    d->buffer[d->write_head] = input + (feedback * current);
    d->write_head = (d->write_head + 1) % d->buffer_size;
}

float delay_line_process(delay_line_t* d, float input, float feedback)
{
    if(d->is_bypassed)
    {
        return input;
    }

    // Smooth the read head position
    d->read_head = d->read_head * d->smoothing + 
                   d->target_read_head * (1.0f - d->smoothing);

    // Keep read_head in valid range
    while(d->read_head < 0)
        d->read_head += d->buffer_size;
    while(d->read_head >= d->buffer_size)
        d->read_head -= d->buffer_size;

    float output = delay_line_read_interpolated(d);
    delay_line_write(d, input, feedback);
    
    // Update read head for next sample
    d->read_head = fmodf(d->read_head + 1, d->buffer_size);
    
    return output;
}