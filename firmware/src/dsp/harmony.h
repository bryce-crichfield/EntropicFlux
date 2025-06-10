#pragma once

#include <stdlib.h>

typedef struct {
    u_int8_t num_voices;
    int8_t voices[3];  // Array to store intervals, sized for largest possible chord
} harmony_t;

// Define common chord harmonies
static const harmony_t MAJOR_TRIAD = {
    .num_voices = 2,
    .voices = {4, 7}  // Major third and perfect fifth
};

static const harmony_t MINOR_TRIAD = {
    .num_voices = 2,
    .voices = {3, 7}  // Minor third and perfect fifth
};

static const harmony_t DIMINISHED_TRIAD = {
    .num_voices = 2,
    .voices = {3, 6}  // Minor third and diminished fifth
};

static const harmony_t AUGMENTED_TRIAD = {
    .num_voices = 2,
    .voices = {4, 8}  // Major third and augmented fifth
};

static const harmony_t MAJOR_SEVENTH = {
    .num_voices = 1,
    .voices = {7}  // Major third, perfect fifth, major seventh
};

static const harmony_t DOMINANT_SEVENTH = {
    .num_voices = 3,
    .voices = {4, 7, 10}  // Major third, perfect fifth, minor seventh
};

// Adding the requested additional harmonies
static const harmony_t PERFECT_FIFTH = {
    .num_voices = 1,
    .voices = {7}  // Perfect fifth only
};


static const harmony_t PERFECT_FOURTH = {
    .num_voices = 1,
    .voices = {5}  // Perfect fourth only
};

static const harmony_t MAJOR_NINTH = {
    .num_voices = 1,
    .voices = {14}  // Major third, perfect fifth, major ninth
};

static const harmony_t MINOR_SIXTH = {
    .num_voices = 1,
    .voices = {8}  // Minor third and minor sixth
};

static const harmony_t MAJOR_SIXTH = {
    .num_voices = 1,
    .voices = {9}  // Major third and major sixth
};

static const harmony_t OCTAVE_UP = {
    .num_voices = 1,
    .voices = {12}  // Octave up
};

static const harmony_t OCTAVE_DOWN = {
    .num_voices = 1,
    .voices = {-12}  // Octave down
};


static const harmony_t SHIMMER = {
    .num_voices = 2,
    .voices = {12, 24}  // Perfect fifth and octave up
};