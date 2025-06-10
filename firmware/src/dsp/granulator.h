#pragma once

#include <stdlib.h>
#include <math.h>

#include "../platform/circular_buffer.h"
#include "../platform/memory.h"

typedef enum
{
    GRAIN_SHAPE_NORMAL,
    GRAIN_SHAPE_TRIANGULAR,
    GRAIN_SHAPE_HANN,
    GRAIN_SHAPE_GAUSSIAN
} grain_shape_t;

float grain_shape_value(grain_shape_t shape, float position)
{
    switch(shape)
    {
        case GRAIN_SHAPE_NORMAL: return 1.0f;
        case GRAIN_SHAPE_TRIANGULAR:
            return 1.0f - fabsf(2.0f * position - 1.0f);
        case GRAIN_SHAPE_HANN:
            return 0.5f * (1.0f - cosf(2.0f * M_PI * position));
        case GRAIN_SHAPE_GAUSSIAN: return expf(-0.5f * position * position);
    }
    return 0.0f;
}

typedef struct
{
    float         position;
    float         speed;
    float         amplitude;
    float         duration;
    size_t        is_active;
    size_t        is_reverse;
    grain_shape_t shape;
} grain_t;

typedef struct
{
    circular_buffer_t audio_buffer;
    circular_buffer_t frozen_buffer;
    grain_t          *grain_buffer;
    size_t            grain_count;
    size_t            buffer_position;
    size_t            grain_counter;
    float             grain_duration;
    float             grain_speed;
    float             grain_density;
    float             grain_amplitude;
    float             grain_overlap;
    grain_shape_t     grain_shape;
    size_t            sample_rate;
    size_t            is_frozen;
    size_t            is_reverse;
} granulator_t;

void granulator_init(granulator_t *g, size_t grain_count, size_t sample_rate)
{
    g->grain_count  = grain_count;
    g->sample_rate  = sample_rate;
    g->grain_buffer = (grain_t *)allocate(grain_count * sizeof(grain_t));
    circular_buffer_init(&g->audio_buffer, sample_rate);
    circular_buffer_init(&g->frozen_buffer, sample_rate);
    g->buffer_position = 0;
    g->grain_counter   = 0;
    g->grain_duration  = 0.1f;
    g->grain_speed     = 1.0f;
    g->grain_density   = 10.0f;
    g->grain_amplitude = 0.5f;
    g->grain_overlap   = 0.5f;
    g->grain_shape     = GRAIN_SHAPE_GAUSSIAN;
    g->is_frozen       = 0;
    g->is_reverse      = 0;

    for(size_t i = 0; i < grain_count; i++)
    {
        g->grain_buffer[i] = (grain_t){0}; // Zero-initialize all fields
    }
}

void granulator_generate(granulator_t *g)
{
    for(size_t i = 0; i < g->grain_count; i++)
    {
        grain_t *grain = &g->grain_buffer[i];
        if(!grain->is_active)
        {
            grain->position
                = ((float)rand() / RAND_MAX) * (float)g->audio_buffer.size;
            grain->amplitude
                = g->grain_amplitude; // grain->speed = 1.0f + (2.0f * ((float)rand() / RAND_MAX) - 1.0f);
            grain->speed      = g->grain_speed;
            grain->amplitude  = g->grain_amplitude * ((float)rand() / RAND_MAX);
            grain->duration   = g->grain_duration;
            grain->is_active  = 1;
            grain->is_reverse = g->is_reverse;
            grain->shape      = g->grain_shape;
            break;
        }
    }
}


void granulator_freeze(granulator_t *g)
{
    for(size_t i = 0; i < g->audio_buffer.size; i++)
    {
        float sample = circular_buffer_read(&g->audio_buffer, i);
        circular_buffer_write(&g->frozen_buffer, sample);
    }
}

float granulator_process(granulator_t *g, float input)
{
    if(!g->is_frozen)
    {
        circular_buffer_write(&g->audio_buffer, input);
        g->buffer_position = (g->buffer_position + 1) % g->audio_buffer.size;
    }

    float output = 0.0f;
    if(++g->grain_counter
       >= (g->sample_rate / g->grain_density) / g->grain_overlap)
    {
        g->grain_counter = 0;
        granulator_generate(g);
    }

    for(size_t i = 0; i < g->grain_count; i++)
    {
        grain_t *grain = &g->grain_buffer[i];
        if(grain->is_active)
        {
            float  phase  = grain->position / g->audio_buffer.size;
            float  window = grain_shape_value(grain->shape, phase);
            size_t pos
                = grain->is_reverse
                      ? (size_t)(g->audio_buffer.size - grain->position - 1)
                            % g->audio_buffer.size
                      : (size_t)grain->position % g->audio_buffer.size;


            float sample
                = g->is_frozen
                      ? circular_buffer_interpolate(&g->frozen_buffer, pos)
                      : circular_buffer_interpolate(&g->audio_buffer, pos);

            output += sample * window * grain->amplitude;
            grain->position += grain->speed;
            grain->is_active = grain->position < g->audio_buffer.size;
        }
    }

    if(g->is_frozen)
    {
        output += input;
    }

    return output;
}