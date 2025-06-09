#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

typedef struct
{
    float *buffer;
    size_t size;
    size_t write_pos;
    size_t read_pos;
} circular_buffer_t;

void circular_buffer_init(circular_buffer_t *cb, size_t size)
{
    cb->buffer = (float *)allocate(size * sizeof(float));
    zero_memory(cb->buffer, size * sizeof(float));
    cb->size      = size;
    cb->write_pos = 0;
}

void circular_buffer_fill(circular_buffer_t *cb, float value)
{
    for(size_t i = 0; i < cb->size; i++)
    {
        cb->buffer[i] = value;
    }
}

void circular_buffer_write(circular_buffer_t *cb, float value)
{
    cb->buffer[cb->write_pos] = value;
    cb->write_pos             = (cb->write_pos + 1) % cb->size;

}

void circular_buffer_write_at(circular_buffer_t *cb, float value, size_t position)
{
    cb->buffer[position % cb->size] = value;
}

float circular_buffer_read(circular_buffer_t *cb, size_t position) {
    if (!cb->buffer || position >= cb->size) return 0.0f;

    return cb->buffer[position % cb->size];
}

float circular_buffer_interpolate(circular_buffer_t *cb, float position)
{
    size_t pos1 = (size_t)position;
    size_t pos2 = (pos1 + 1) % cb->size;
    float  frac = position - (float)pos1;
    float  s1   = circular_buffer_read(cb, pos1);
    float  s2   = circular_buffer_read(cb, pos2);
    return s1 * (1.0f - frac) + s2 * frac;
}