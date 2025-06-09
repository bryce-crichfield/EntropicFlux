// #pragma once

// #include <math.h>
// #include <string.h>

// #define FDN_SIZE 8
// #define MAX_DELAY_SAMPLES 48000  // 1 second at 48kHz
// #define FDN_ALLPASS_COUNT 4
// static const float FDN_ALLPASS_DELAYS_S[FDN_ALLPASS_COUNT] = {0.013f, 0.023f, 0.037f, 0.019f};

// typedef struct {
//     float* delay_lines[FDN_SIZE];
//     size_t delay_lengths[FDN_SIZE];
//     size_t write_pos[FDN_SIZE];
//     float feedback_gain;
//     float input_gains[FDN_SIZE];
//     float output_gains[FDN_SIZE];
//     size_t sample_rate;
//     biquad_t filters[FDN_SIZE][3];  // Add filters to eliminate resonances form the feedback
// } fdn_t;

// static const float PRIME_DELAYS_MS[] = {13.7f, 19.2f, 26.9f, 37.6f};
void hadamard_transform(float* buffer, int size) {
    for (int step = 1; step < size; step <<= 1) {
        for (int i = 0; i < size; i += 2 * step) {
            for (int j = i; j < i + step; j++) {
                float a = buffer[j];
                float b = buffer[j + step];
                buffer[j] = a + b;
                buffer[j + step] = a - b;
            }
        }
    }

    // Normalize
    float norm = 1.0f / sqrtf(size);
    for (int i = 0; i < size; i++) {
        buffer[i] *= norm;
    }
}

// void fdn_init(fdn_t* fdn, size_t sample_rate) {
//     fdn->sample_rate = sample_rate;

//     // Delay lines in ms: 13.7, 19.2, 26.9, 37.6
//     for (int i = 0; i < FDN_SIZE; i++) {
//         size_t samples = (size_t)(PRIME_DELAYS_MS[i] * (sample_rate / 1000.0f));
//         fdn->delay_lengths[i] = samples;
//         fdn->delay_lines[i] = (float*)allocate(samples * sizeof(float));

//         for (size_t j = 0; j < samples; j++) {
//             fdn->delay_lines[i][j] = 0.0f;
//         }
//         fdn->write_pos[i] = 0;
//     }

//     // fdn->feedback_gain = 0.97f;
//     // for (int i = 0; i < FDN_SIZE; i++) {
//     //     fdn->input_gains[i] = (i % 2 == 0) ? 0.7f : -0.7f;
//     //     fdn->output_gains[i] = 0.5f;
//     // }

//     // // init the filters
//     // for (int i = 0; i < FDN_SIZE; i++) {
//     //     for (int j = 0; j < 3; j++) {
//     //         biquad_init(&fdn->filters[i][j], FILTER_LOWPASS, 20000, 0.7f, 0.1, sample_rate);
//     //     }
//     // }
// }

// float fdn_process(fdn_t* fdn, float input) {
//     float output = 0.0f;
//     float feedback = 0.97f;  // Test with basic feedback

//     for (int i = 0; i < FDN_SIZE; i++) {
//         int read_pos = (int)fdn->write_pos[i] - (int)fdn->delay_lengths[i];
//         while (read_pos < 0) read_pos += fdn->delay_lengths[i];

//         float delayed = fdn->delay_lines[i][read_pos];
//         fdn->delay_lines[i][fdn->write_pos[i]] = input + (delayed * feedback);
//         fdn->write_pos[i] = (fdn->write_pos[i] + 1) % fdn->delay_lengths[i];

//         output += delayed;
//     }

//     return output * 0.25f;
//     // float output = input;
//     // float delay_outputs[FDN_SIZE];
//     // float mixed[FDN_SIZE];

//     // // // Read from delays
//     // for (int i = 0; i < FDN_SIZE; i++) {
//     //     int read_pos = (int)fdn->write_pos[i] - (int)fdn->delay_lengths[i];
//     //     while (read_pos < 0)
//     //         read_pos += fdn->delay_lengths[i];
//     //     delay_outputs[i] = fdn->delay_lines[i][read_pos];
//     //     mixed[i] = delay_outputs[i] + (input * fdn->input_gains[i]);
//     //     output += delay_outputs[i];
//     // }

//     // // Apply Hadamard
//     // hadamard_transform(mixed, FDN_SIZE);

//     // // Write back with feedback
//     // for (int i = 0; i < FDN_SIZE; i++) {
//     //     float feedback = mixed[i] * fdn->feedback_gain;
//     //     for (int j = 0; j < 3; j++) {
//     //         feedback = biquad_process(&fdn->filters[i][j], feedback);
//     //     }
//     //     // fdn->delay_lines[i][fdn->write_pos[i]] = input + (mixed[i] * fdn->feedback_gain);
//     //     fdn->delay_lines[i][fdn->write_pos[i]] = feedback;
//     //     fdn->write_pos[i] = (fdn->write_pos[i] + 1) % fdn->delay_lengths[i];
//     //     output += delay_outputs[i] * fdn->output_gains[i];
//     // }

//     // return output;
// }

// void fdn_set_damping(fdn_t* fdn, float damping) {
//     //     fdn->damping = fmaxf(0.0f, fminf(damping, 1.0f));

//     //     // Map damping 0-1 to frequency range 20000-2000 Hz
//     //     float freq = 2000.0f + (18000.0f * (1.0f - fdn->damping));

//     //     for (int i = 0; i < FDN_SIZE; i++) {
//     //         biquad_set_frequency(&fdn->filters[i][0], freq);
//     //     }
// }

// void fdn_set_decay(fdn_t* fdn, float decay_seconds) {
//     decay_seconds *= 1.2f;
//     // Calculate based on average delay time
//     float avg_delay_time = 0.0f;
//     for (int i = 0; i < FDN_SIZE; i++) {
//         avg_delay_time += (float)fdn->delay_lengths[i] / fdn->sample_rate;
//     }
//     avg_delay_time /= FDN_SIZE;

//     // RT60 decay calculation
//     float feedback = powf(0.003f, (avg_delay_time / decay_seconds) * 0.5f);
//     fdn->feedback_gain = fminf(fmaxf(feedback, 0.0f), 0.998f);
// }

// void fdn_set_modulation_rate(fdn_t* fdn, float rate) {
//     // rate = powf(rate, 2.0f);
//     // float step_size = 0.001f + (rate * 0.04f);  // Reduced step size range
//     // for (int i = 0; i < FDN_SIZE; i++) {
//     //     noise_set_step_size(&fdn->noise[i], step_size);
//     // }
// }

#pragma once

#include "delay.h"
#include "allpass.h"
#include "../platform/circular_buffer.h"

#define FDN_SIZE 8
#define MAX_DELAY_SAMPLES 48000
#define FDN_ALLPASS_COUNT 4
static const float FDN_ALLPASS_DELAYS_S[FDN_ALLPASS_COUNT] = {0.013f, 0.023f, 0.037f, 0.019f};
typedef struct {
    circular_buffer_t delays[FDN_SIZE];
    float* delay_outputs[FDN_SIZE];
    float* feedback_outputs[FDN_SIZE];
    float feedback_gain;
    float input_gains[FDN_SIZE];
    float output_gains[FDN_SIZE];
    size_t sample_rate;
    size_t delay_lengths[FDN_SIZE];

    allpass_t allpasses[FDN_SIZE];
} fdn_t;

void fdn_init(fdn_t* fdn, size_t sample_rate) {
    fdn->sample_rate = sample_rate;
    static const float PRIME_DELAYS_MS[] = {13.7f, 19.2f, 26.9f, 37.6f};

    fdn->feedback_gain = 0.95f;
    for (int i = 0; i < FDN_SIZE; i++) {
        fdn->delay_lengths[i] = (size_t)(PRIME_DELAYS_MS[i % 4] * (sample_rate / 1000.0f));
        circular_buffer_init(&fdn->delays[i], fdn->delay_lengths[i]);
        fdn->input_gains[i] = (i % 2 == 0) ? 0.7f : -0.7f;
        fdn->output_gains[i] = 0.5f;
    }

    for (int i = 0; i < FDN_ALLPASS_COUNT; i++) {
        allpass_init(&fdn->allpasses[i], sample_rate);
        allpass_set_delay(&fdn->allpasses[i], FDN_ALLPASS_DELAYS_S[i], sample_rate);
        allpass_set_feedback(&fdn->allpasses[i], 0.5f);
    }
}

float fdn_process(fdn_t* fdn, float input) {
    float output = 0.0f;
    float delay_outputs[FDN_SIZE];
    float delay_inputs[FDN_SIZE];

    // Read outputs
    for (int i = 0; i < FDN_SIZE; i++) {
        size_t read_pos =
            (fdn->delays[i].write_pos + fdn->delay_lengths[i]) % fdn->delay_lengths[i];
        delay_outputs[i] = circular_buffer_read(&fdn->delays[i], read_pos);
    }

    // First mix the delayed signals through Hadamard
    hadamard_transform(delay_outputs, FDN_SIZE);

    // Then apply feedback and input
    for (int i = 0; i < FDN_SIZE; i++) {
        delay_inputs[i] = (delay_outputs[i] * fdn->feedback_gain) + (input * fdn->input_gains[i]);
    }

    // Write to delays
    for (int i = 0; i < FDN_SIZE; i++) {
        circular_buffer_write(&fdn->delays[i], delay_inputs[i]);
        output += delay_outputs[i] * fdn->output_gains[i];
    }

    // Apply allpasses in parallel
    float allpass_out = 0.0f;
    for (int i = 0; i < FDN_ALLPASS_COUNT; i++) {
        allpass_out += allpass_process(&fdn->allpasses[i], output);
    }
    output = allpass_out;


    return tanh(output * 0.5f);
}

void fdn_set_decay(fdn_t* fdn, float decay_seconds) {
    decay_seconds *= 1.2f;
    // Calculate based on average delay time
    float avg_delay_time = 0.0f;
    for (int i = 0; i < FDN_SIZE; i++) {
        avg_delay_time += (float)fdn->delay_lengths[i] / fdn->sample_rate;
    }
    avg_delay_time /= FDN_SIZE;

    // RT60 decay calculation
    float feedback = powf(0.003f, (avg_delay_time / decay_seconds) * 0.5f);
    fdn->feedback_gain = fminf(fmaxf(feedback, 0.0f), 0.998f);
}