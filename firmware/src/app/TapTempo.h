// #pragma once
//
// #include "daisy_seed.h"
// #include <stdint.h>
//
// class TapTempo {
//     private:
//         daisy::Switch tap;
//         uint32_t lastTapTime;
//         float currentTempo;
//         bool lastState;
//         static const uint32_t TAP_TIMEOUT = 2000;  // Reset after 2 seconds
//         static const uint32_t MIN_TAP_TIME = 100;  // Minimum tap interval in ms
//         bool wasUpdated;
//
//     public:
//         TapTempo() : lastTapTime(0), currentTempo(120.0f), lastState(false) {}
//
//         void Init(dsy_gpio_pin pin) {
//             tap.Init(pin, 1000.0f);
//             lastTapTime = daisy::System::GetNow();
//         }
//
//         void Process() {
//             tap.Debounce();
//             bool currentState = tap.Pressed();
//             uint32_t currentTime = daisy::System::GetNow();
//             wasUpdated = false;
//             // Detect falling edge (tap release)
//             if (currentState && !lastState) {
//                 wasUpdated = true;
//                 // If it's been too long, reset
//                 if ((currentTime - lastTapTime) > TAP_TIMEOUT) {
//                     lastTapTime = currentTime;
//                 }
//                 // Calculate new tempo
//                 else {
//                     float timeDiff = (currentTime - lastTapTime) / 1000.0f;
//                     if (timeDiff > (MIN_TAP_TIME / 1000.0f)) {
//                         float newTempo = 60.0f / timeDiff;
//                         // Smooth the tempo changes
//                         currentTempo = currentTempo * 0.5f + newTempo * 0.5f;
//                         lastTapTime = currentTime;
//                     }
//                 }
//             }
//             lastState = currentState;
//         }
//
//         float GetDelayTime() {
//             return 60.0f / currentTempo;
//         }
//
//         float GetTempo() {
//             return currentTempo;
//         }
//
//         bool WasUpdated() {
//             return wasUpdated;
//         }
//     };