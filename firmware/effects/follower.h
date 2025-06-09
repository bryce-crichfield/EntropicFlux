#ifndef FOLLOWER_H
#define FOLLOWER_H

#include <math.h>

typedef struct {
   float attack_coef;
   float release_coef;
   float envelope;
} follower_t;

void follower_init(follower_t *follower, float sample_rate) {
   float attack_time = 0.01f;  // 10ms attack
   float release_time = 0.1f;  // 100ms release
   
   follower->attack_coef = expf(-1.0f / (sample_rate * attack_time));
   follower->release_coef = expf(-1.0f / (sample_rate * release_time));
   follower->envelope = 0.0f;
}

void follower_set_attack(follower_t *follower, float attack_time, float sample_rate) {
   follower->attack_coef = expf(-1.0f / (sample_rate * attack_time));
}

void follower_set_release(follower_t *follower, float release_time, float sample_rate) {
   follower->release_coef = expf(-1.0f / (sample_rate * release_time));
}

float follower_process(follower_t *follower, float input) {
   float abs_input = fabsf(input);
   
   if (abs_input > follower->envelope) {
       follower->envelope = abs_input + follower->attack_coef * 
                          (follower->envelope - abs_input);
   } else {
       follower->envelope = abs_input + follower->release_coef * 
                          (follower->envelope - abs_input);
   }
   
   return follower->envelope;
}

#endif