#include <stdint.h>
#include <string.h>
#include <math.h>

#define MAX_PATTERN_LENGTH 32

typedef struct {
    float base_frequency;
    float tempo_hz;
    uint32_t current_step;  // Current position in the pattern
    float time_per_note;
    float time_elapsed;
    float sample_rate;
    
    // Pattern-related members
    int8_t pattern[MAX_PATTERN_LENGTH];  // Store semitone intervals, -1 for silence
    uint8_t pattern_length;
} arpeggiator_t;

// Helper function to convert semitones to frequency multiplier
static float semitone_to_multiplier(int8_t semitones) {
    return powf(2.0f, semitones / 12.0f);
}

// Parse pattern string into semitone intervals
void arpeggiator_set_pattern(arpeggiator_t *arp, const char *pattern) {
    arp->pattern_length = 0;
    
    const char *ptr = pattern;
    while (*ptr && arp->pattern_length < MAX_PATTERN_LENGTH) {
        if (*ptr >= '0' && *ptr <= '9') {
            arp->pattern[arp->pattern_length++] = *ptr - '0';
        } else if (*ptr == '-') {
            arp->pattern[arp->pattern_length++] = -1;  // -1 represents silence
        }
        ptr++;
    }
}

void arpeggiator_init(arpeggiator_t *arp, float sample_rate, float base_freq, float tempo_bpm) {
    arp->sample_rate = sample_rate;
    arp->base_frequency = base_freq;
    arp->tempo_hz = tempo_bpm / 60.0f;
    arp->current_step = 0;
    arp->time_per_note = 1.0f / arp->tempo_hz;
    arp->time_elapsed = 0.0f;
    
    // Initialize with a default pattern
    const char *default_pattern = "0-0-4-4";
    arpeggiator_set_pattern(arp, default_pattern);
}

// Returns the current frequency (0.0f for silence)
float arpeggiator_process(arpeggiator_t *arp) {
    arp->time_elapsed += 1.0f / arp->sample_rate;
    
    if (arp->time_elapsed >= arp->time_per_note) {
        arp->time_elapsed -= arp->time_per_note;
        arp->current_step = (arp->current_step + 1) % arp->pattern_length;
    }
    
    int8_t semitones = arp->pattern[arp->current_step];
    if (semitones >= 0) {
        return arp->base_frequency * semitone_to_multiplier(semitones);
    }
    return 0.0f;  // Return 0 Hz for silence
}