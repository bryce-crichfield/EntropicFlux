#pragma once

#include "daisy_seed.h"


class Button {
private:
   daisy::Switch button;
   bool lastState;
   uint32_t lastPressTime;
   static const uint32_t DEBOUNCE_DELAY = 125;

public:
   Button() : lastState(false), lastPressTime(0) {}

   void Init(dsy_gpio_pin pin) {
       button.Init(pin, 1000.0f);
   }

   bool checkButton() {
       button.Debounce();
       bool currentState = button.Pressed();
       uint32_t currentTime = daisy::System::GetNow();

       if (currentState != lastState) {
           if (currentState && (currentTime - lastPressTime) > DEBOUNCE_DELAY) {
               lastPressTime = currentTime;
               lastState = currentState;
               return true;
           }
           lastState = currentState;
       }
       return false;
   }
};