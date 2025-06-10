#pragma once

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define MAX_DIFFUSER_TAPS 50
#define BATCH_SIZE 1

typedef struct {
    float* buffer;           // Circular buffer for delay line
    size_t buffer_size;      
    int write_index;         // Current write position
    float tap_gains[MAX_DIFFUSER_TAPS];
    float tap_positions[MAX_DIFFUSER_TAPS];
    float tap_lengths[MAX_DIFFUSER_TAPS];
    float seed_values[MAX_DIFFUSER_TAPS * 2];
    int seed;
    size_t tap_count;
    float tap_length;
    float gain;
    float decay;
} diffuser_t;

void diffuser_init(diffuser_t* m, int delay_buffer_size) {
    m->buffer = (float*) malloc(delay_buffer_size * sizeof(float));
    memset(m->buffer, 0, delay_buffer_size * sizeof(float));
    m->buffer_size = delay_buffer_size;
    m->write_index = 0;
    m->tap_count = 8;  // Increase from 1 to 8 taps
    m->tap_length = 32;  // Increase tap length too
    m->gain = 0.5f;
    m->decay = 0.1f;  // Add some decay
    m->seed = 12345;
    update_seeds(m);
}

static float generate_random(float* seed_values, int* index) {
    return seed_values[(*index)++];
}

void update_taps(diffuser_t* m) {
    int seed_index = 0;
    
    // Make sure tap_length is at least tap_count
    if (m->tap_length < m->tap_count)
        m->tap_length = m->tap_count;

    // Calculate tap count factor for volume adjustment
    float tap_count_factor = 1.0f / (1.0f + sqrtf(m->tap_count / (float)MAX_DIFFUSER_TAPS));

    // Randomly space out taps
    float sum_lengths = 0.0f;
    for (size_t i = 0; i < m->tap_count; i++) {
        float val = 0.1f + generate_random(m->seed_values, &seed_index);
        m->tap_lengths[i] = val;
        sum_lengths += val;
    }

    float scale_length = m->tap_length / sum_lengths;
    m->tap_positions[0] = 0;
    
    // Position taps based on scaled random lengths
    for (size_t i = 1; i < m->tap_count; i++) {
        m->tap_positions[i] = m->tap_positions[i - 1] + 
                    (int)(m->tap_lengths[i] * scale_length);
    }

    float last_tap_pos = m->tap_positions[m->tap_count - 1];
    
    // Calculate gains with decay
    for (size_t i = 0; i < m->tap_count; i++) {
        float g = powf(10, -m->decay * 2 * m->tap_positions[i] / 
                      (float)(last_tap_pos + 1));
        float tap = (2 * generate_random(m->seed_values, &seed_index) - 1) * 
                   tap_count_factor;
        m->tap_gains[i] = tap * g * m->gain;
    }

    // Set initial tap gain
    m->tap_gains[0] = (1.0f - m->gain);
}

void update_seeds(diffuser_t* m) {
    for (int i = 0; i < MAX_DIFFUSER_TAPS * 2; i++) {
        m->seed_values[i] = (float)rand() / RAND_MAX;
    }
    update_taps(m);
    
    printf("After initialization:\n");
    for (size_t i = 0; i < m->tap_count; i++) {
        printf("Tap %zu: position=%f gain=%f\n", 
               i, m->tap_positions[i], m->tap_gains[i]);
    }
}

float diffuser_process(diffuser_t* m, float input) {
    // Write input to buffer
    m->buffer[m->write_index] = input;
    
    float output = 0.0f;
    
    // Process each tap
    for (size_t i = 0; i < m->tap_count; i++) {
        // Calculate read position for this tap
        int read_pos = m->write_index - (int)m->tap_positions[i];
        if (read_pos < 0) {
            read_pos += m->buffer_size;
        }
        
        // Add tap contribution
        output += m->buffer[read_pos] * m->tap_gains[i];
    }
    
    // Increment write position with wrap
    m->write_index = (m->write_index + 1) % m->buffer_size;
    
    return output;
}

// Setter functions
void diffuser_set_tap_count(diffuser_t* m, int count) {
    m->tap_count = count > 1 ? count : 1;
    update_taps(m);
}

void diffuser_set_tap_length(diffuser_t* m, int length) {
    m->tap_length = length;
    update_taps(m);
}

void diffuser_set_decay(diffuser_t* m, float decay) {
    m->decay = decay;
    update_taps(m);
}

void diffuser_set_gain(diffuser_t* m, float gain) {
    m->gain = gain;
    update_taps(m);
}

void diffuser_set_seed(diffuser_t* m, int seed) {
    m->seed = seed;
    update_seeds(m);
}