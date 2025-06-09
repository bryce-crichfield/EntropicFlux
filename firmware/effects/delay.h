#pragma once

#include <stdlib.h>
#include <stddef.h>
#include "../platform/memory.h"

typedef struct
{
    int    buffer_size;
    float* buffer;
    int    write_head;
    int    read_head;
    size_t is_bypassed;                 // used for cases where delay is 0
} delay_t;

void delay_init(delay_t* d, int buffer_size)
{
    d->buffer_size = buffer_size;
    d->buffer      = (float*)allocate(buffer_size * sizeof(float));
    zero_memory(d->buffer, buffer_size * sizeof(float));
    d->write_head  = 0;
    d->read_head   = 0;
    d->is_bypassed = 0;
}

void delay_set_delay_samples(delay_t* d, float delay)
{ // Changed int to float
    if(delay <= 0)
    {
        d->is_bypassed = 1;
        return;
    }

    d->is_bypassed = 0;

    float read_pos = d->write_head - delay;
    while(read_pos < 0)
        read_pos += d->buffer_size;
    d->read_head = (int)read_pos; // Cast back to int for buffer access
}

void delay_set_delay_time(delay_t* d, float delay_time, float sample_rate)
{
    delay_set_delay_samples(d, delay_time * sample_rate);
}

float delay_tap(delay_t* d)
{
    return d->buffer[d->read_head];
}

void delay_write(delay_t* d, float input, float feedback)
{
    d->buffer[d->write_head] = input + (feedback * delay_tap(d));
    d->write_head            = (d->write_head + 1) % d->buffer_size;
}

void delay_write_no_feedback(delay_t* d, float input) {
    d->buffer[d->write_head] = input;
    d->write_head = (d->write_head + 1) % d->buffer_size;
}

float delay_read(delay_t* d)
{
    float output = d->buffer[d->read_head];
    d->read_head = (d->read_head + 1) % d->buffer_size;
    return output;
}

float delay_process(delay_t* d, float input, float feedback)
{
    if(d->is_bypassed)
    {
        return input;
    }

    float output = delay_read(d); // Use delay_read instead of delay_tap
    delay_write(d, input, feedback);
    return output;
}

float delay_get(delay_t* d, int offset)
{
    int index = (d->read_head + offset) % d->buffer_size;
    return d->buffer[index];
}

float delay_read_interpolated(delay_t* d)
{
    float frac = d->read_head - floorf(d->read_head);
    float s1   = delay_read(d);
    d->read_head += 1.0f;
    float s2 = delay_read(d);
    d->read_head -= 1.0f;
    return s1 + frac * (s2 - s1);
}