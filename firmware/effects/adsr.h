#include <stdbool.h>

typedef enum {
    ADSR_IDLE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
} adsr_stage_t;

typedef struct {
    float sample_rate;
    
    // Time settings (in seconds)
    float attack_time;
    float decay_time;
    float sustain_level;  // 0.0 to 1.0
    float release_time;
    
    // Internal state
    adsr_stage_t current_stage;
    float current_level;
    float rate;          // Current rate of change
    bool gate;          // Whether note is currently held
} adsr_t;

void adsr_init(adsr_t *adsr, float sample_rate) {
    adsr->sample_rate = sample_rate;
    adsr->attack_time = 0.1f;
    adsr->decay_time = 0.1f;
    adsr->sustain_level = 0.7f;
    adsr->release_time = 0.2f;
    
    adsr->current_stage = ADSR_IDLE;
    adsr->current_level = 0.0f;
    adsr->rate = 0.0f;
    adsr->gate = false;
}

void adsr_set_params(adsr_t *adsr, float attack, float decay, float sustain, float release) {
    adsr->attack_time = attack;
    adsr->decay_time = decay;
    adsr->sustain_level = sustain;
    adsr->release_time = release;
}

void adsr_gate_on(adsr_t *adsr) {
    adsr->gate = true;
    adsr->current_stage = ADSR_ATTACK;
    adsr->rate = 1.0f / (adsr->attack_time * adsr->sample_rate);
}

void adsr_gate_off(adsr_t *adsr) {
    adsr->gate = false;
    adsr->current_stage = ADSR_RELEASE;
    adsr->rate = adsr->current_level / (adsr->release_time * adsr->sample_rate);
}

float adsr_process(adsr_t *adsr) {
    switch (adsr->current_stage) {
        case ADSR_IDLE:
            return 0.0f;
            
        case ADSR_ATTACK:
            adsr->current_level += adsr->rate;
            if (adsr->current_level >= 1.0f) {
                adsr->current_level = 1.0f;
                adsr->current_stage = ADSR_DECAY;
                adsr->rate = (1.0f - adsr->sustain_level) / (adsr->decay_time * adsr->sample_rate);
            }
            break;
            
        case ADSR_DECAY:
            adsr->current_level -= adsr->rate;
            if (adsr->current_level <= adsr->sustain_level) {
                adsr->current_level = adsr->sustain_level;
                adsr->current_stage = ADSR_SUSTAIN;
            }
            break;
            
        case ADSR_SUSTAIN:
            adsr->current_level = adsr->sustain_level;
            if (!adsr->gate) {
                adsr->current_stage = ADSR_RELEASE;
                adsr->rate = adsr->current_level / (adsr->release_time * adsr->sample_rate);
            }
            break;
            
        case ADSR_RELEASE:
            adsr->current_level -= adsr->rate;
            if (adsr->current_level <= 0.0f) {
                adsr->current_level = 0.0f;
                adsr->current_stage = ADSR_IDLE;
            }
            break;
    }
    
    return adsr->current_level;
}