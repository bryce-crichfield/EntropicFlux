#pragma once

#include <math.h>

typedef enum {
    SOFT_CLIP,
    HARD_CLIP,
    TANH,
    SINE_FOLD,
    CUBIC
} saturation_type_t;

typedef struct {
    saturation_type_t type;
    float drive;      // Amount of saturation/distortion (1.0 = none, >1.0 = more)
    float bias;       // DC offset before saturation
    float output_gain; // Post-saturation gain
} saturator_t;

void saturator_init(saturator_t *sat) {
    sat->type = SOFT_CLIP;
    sat->drive = 1.0f;
    sat->bias = 0.0f;
    sat->output_gain = 1.0f;
}

void saturator_set_type(saturator_t *sat, saturation_type_t type) {
    sat->type = type;
}

void saturator_set_drive(saturator_t *sat, float drive) {
    sat->drive = drive;
}

void saturator_set_bias(saturator_t *sat, float bias) {
    sat->bias = bias;
}

void saturator_set_output_gain(saturator_t *sat, float gain) {
    sat->output_gain = gain;
}

float saturator_process(saturator_t *sat, float input) {
    float output = 0.0f;
    
    // Apply drive and bias
    float x = (input * sat->drive) + sat->bias;
    
    switch(sat->type) {
        case SOFT_CLIP:
            // Smooth sigmoid-like clipping
            if (x > 1.0f) {
                output = 1.0f - 1.0f/expf(x - 1.0f);
            } else if (x < -1.0f) {
                output = -1.0f + 1.0f/expf(-x - 1.0f);
            } else {
                output = x;
            }
            break;
            
        case HARD_CLIP:
            // Simple hard clipping
            if (x > 1.0f) output = 1.0f;
            else if (x < -1.0f) output = -1.0f;
            else output = x;
            break;
            
        case TANH:
            // Hyperbolic tangent saturation
            output = tanhf(x);
            break;
            
        case SINE_FOLD:
            // Sine waveshaping for more extreme effects
            output = sinf(M_PI * x * 0.5f);
            break;
            
        case CUBIC:
            // Cubic soft clipping
            if (x > 1.0f) output = 2.0f/3.0f;
            else if (x < -1.0f) output = -2.0f/3.0f;
            else output = x - (x * x * x)/3.0f;
            break;
    }
    
    // Apply output gain
    return output * sat->output_gain;
}