#pragma once

#ifndef NUM_ALLPASS
#define NUM_ALLPASS 6
#endif

#ifndef NUM_COMB
#define NUM_COMB 8
#endif

typedef struct
{
    float allpass_delays[NUM_ALLPASS]; // Delay times for allpass filters
    float allpass_gains[NUM_ALLPASS];  // Feedback gains for allpass filters
    float comb_delays[NUM_COMB];       // Delay times for comb filters
    float comb_gains[NUM_COMB];        // Feedback gains for comb filters
    float room_size;
    float damping;
} reverb_model_t;

static const reverb_model_t REVERB_HALL = {
    .allpass_delays = {
        0.0155f,
        0.0257f,
        0.03543f,
        0.01753f,
        0.02835f,
        0.031626f
    },
    .allpass_gains = {
        0.60f, 
        0.55f, 
        0.50f,
        0.60f, 
        0.55f, 
        0.50f
    },
    .comb_delays    = {
        0.0531f, 
        0.0671f, 
        0.0831f, 
        0.0971f, 
        0.1071f, 
        0.1271f, 
        0.1431f, 
        0.1491f
    },
    .room_size = 0.7f,
    .damping   = 0.7f
};
