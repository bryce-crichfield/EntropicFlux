#pragma once
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include "allpass.h"
#include "comb.h"
#include "reverb_model.h"
#include "noise.h"
#include "../platform/util.h"

#define NUM_ALLPASS 2
#define NUM_COMB 4

typedef struct
{
    allpass_t      allpass[NUM_ALLPASS];
    comb_t         comb[NUM_COMB];
    reverb_model_t model;
    float room_size;                            // Adds to the delay time of the allpass and comb filters
    float damping;                              // Subtracts from the feedback gain of the comb filters
    float width;                                // Scales the wet signal
    float predelay;                             // Adds more delay to each filter
    size_t sample_rate;
} reverb_t;


void reverb_update_parameters(reverb_t *r)
{
    r->room_size = r->model.room_size;
    r->damping   = r->model.damping;

    // Update allpass filters
    for(int i = 0; i < NUM_ALLPASS; i++)
    {
        float delay = r->model.allpass_delays[i] * (1.0f + r->room_size);
        float gain  = r->model.allpass_gains[i] * (1.0f - r->damping * 0.2f);
        // allpass_set_delay(&r->allpass[i], delay + r->predelay, r->sample_rate);
        allpass_set_feedback(&r->allpass[i], gain);
    }

    // Update comb filters
    for(int i = 0; i < NUM_COMB; i++)
    {
        float delay = r->model.comb_delays[i] * (1.0f + r->room_size);
        float gain  = 0.84f * (1.0f - r->damping * 0.15f);
        comb_set_delay(&r->comb[i], delay + r->predelay, r->sample_rate);
        comb_set_feedback(&r->comb[i], gain);
        comb_set_damping(&r->comb[i], r->damping);
    }
}

void reverb_init(reverb_t *r, size_t sample_rate)
{
    r->sample_rate = sample_rate;
    r->room_size   = 0.25f;
    r->damping     = 0.25f;
    r->width       = 1.0f;
    r->predelay    = 0.0f;

    // Initialize allpass filters
    // In reverb_init:
    for(int i = 0; i < NUM_ALLPASS; i++)
    {
        // allpass_init(&r->allpass[i], sample_rate);
    }

    // Initialize comb filters
    for(int i = 0; i < NUM_COMB; i++)
    {
        comb_init(&r->comb[i], sample_rate);
    }

    reverb_update_parameters(r);
}

float reverb_process(reverb_t *r, float input)
{
    float wet = 0.0f;

    // Process through all comb filters in parallel
    for(int i = 0; i < NUM_COMB; i++)
    {
        wet += comb_process(&r->comb[i], input);
    }
    wet *= 1.0f / NUM_COMB;


    // Process through allpass filters in series
    for(int i = 0; i < NUM_ALLPASS; i++)
    {
        input = allpass_process(&r->allpass[i], input);
    }
    wet += input / NUM_ALLPASS;

    return wet * r->width;
}

// =================================================================================================
// Setters
// =================================================================================================
void reverb_set_model(reverb_t *r, const reverb_model_t *model)
{
    r->model = *model;
    reverb_update_parameters(r);
}

void reverb_set_room_size(reverb_t *r, float size)
{
    r->room_size = clamp(size, 0.0f, 1.0f);
    reverb_update_parameters(r);
}

void reverb_set_damping(reverb_t *r, float damping)
{
    r->damping = clamp(damping, 0.0f, 1.0f);
    reverb_update_parameters(r);
}

void reverb_set_width(reverb_t *r, float width)
{
    r->width = clamp(width, 0.0f, 1.0f);
}

void reverb_set_predelay(reverb_t *r, float predelay)
{
    r->predelay = predelay > 0.0f ? predelay : 0.0f;
    reverb_update_parameters(r);
}