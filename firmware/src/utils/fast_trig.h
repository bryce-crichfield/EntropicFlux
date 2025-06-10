#pragma once

#include <math.h>

#define TABLE_SIZE 1024
#define TABLE_MASK (TABLE_SIZE - 1)

// Add these as global or static variables
float sin_table[TABLE_SIZE];
float cos_table[TABLE_SIZE];

// Initialize lookup tables
void init_trig_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        float angle = (2.0f * M_PI * i) / TABLE_SIZE;
        sin_table[i] = sinf(angle);
        cos_table[i] = cosf(angle);
    }
}

// Lookup functions
float table_sin(float x) {
    float index = (x * TABLE_SIZE) / (2.0f * M_PI);
    int idx = ((int)index) & TABLE_MASK;
    return sin_table[idx];
}

float table_cos(float x) {
    float index = (x * TABLE_SIZE) / (2.0f * M_PI);
    int idx = ((int)index) & TABLE_MASK;
    return cos_table[idx];
}