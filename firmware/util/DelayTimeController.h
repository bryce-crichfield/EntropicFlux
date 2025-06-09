#pragma once

#include "Potentiometer.h"
#include "TapTempo.h"

inline float GetRhythmMultiplier(size_t tapIndex, size_t rhythm) {
    switch (rhythm) {
        case 0: return 1.0f;
        case 1: return (tapIndex % 2 == 0) ? 1.5f : 1.0f;
        case 2: return (tapIndex % 2 == 0) ? 1.333333f : 1.0f;
        case 3: return 0.5f;
        default: return 1.0f;
    }
}

class DelayTimeController {
    private:
        Potentiometer& pot;
        TapTempo& tap;
        float currentDelayTime;
        bool potEnabled;
        static constexpr float POT_THRESHOLD = 0.01f; // Threshold for pot movement detection
    
    public:
        DelayTimeController(Potentiometer& pot, TapTempo& tap) 
            : pot(pot), tap(tap), currentDelayTime(0.5f), potEnabled(true) {}
    
        void Process() {
            pot.Update();
            tap.Process();
    
            // If pot was moved significantly
            if (std::abs(pot.GetValue() - currentDelayTime) > POT_THRESHOLD) {
                potEnabled = true;
                currentDelayTime = pot.GetValue();
            }
            
            // If tap tempo was updated
            if (tap.WasUpdated()) {
                potEnabled = false;
                currentDelayTime = tap.GetDelayTime();
            }
        }
    
        float GetDelayTime() const {
            return currentDelayTime;
        }
    
        bool WasUpdated() const {
            return pot.WasUpdated() || tap.WasUpdated();
        }
    
        bool IsPotEnabled() const {
            return potEnabled;
        }



    };