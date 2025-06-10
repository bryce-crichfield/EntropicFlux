// #pragma once
// #include "daisy_seed.h"
//
// class ShiftRegister {
//    private:
//     daisy::Led ds_led, stcp_led, shcp_led;
//     uint8_t state;
//
//     void pulsePin(daisy::Led &led) {
//         led.Set(true);
//         led.Update();
//         daisy::System::DelayUs(1);
//         led.Set(false);
//         led.Update();
//         daisy::System::DelayUs(1);
//     }
//
//    public:
//     ShiftRegister() : state(0) {}
//
//   void Init() {
//         ds_led.Init(daisy::seed::D14, false);
//         stcp_led.Init(daisy::seed::D13, false);
//         shcp_led.Init(daisy::seed::D12, false);
//
//         ds_led.Set(false);
//         stcp_led.Set(false);
//         shcp_led.Set(false);
//
//         ds_led.Update();
//         stcp_led.Update();
//         shcp_led.Update();
//     }
//
//     void writeBit(bool bit) {
//         ds_led.Set(bit);
//         ds_led.Update();
//         pulsePin(shcp_led);
//     }
//
//     void write(uint8_t data) {
//         state = data;
//         for (int i = 7; i >= 0; i--)
//             writeBit(data & (1 << i));
//         pulsePin(stcp_led);
//     }
//
//     void Set(uint8_t led, bool state) {
//         if (led > 7)
//             return;
//         this->state = state ? (this->state | (1 << led)) : (this->state & ~(1 << led));
//         write(this->state);
//     }
// };