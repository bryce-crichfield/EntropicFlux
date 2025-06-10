//
// Created by BC118580 on 6/9/2025.
//

#ifndef DESKTOPPLATFORM_H
#define DESKTOPPLATFORM_H

namespace Entropic::Flux {
class DesktopPlatform {

};


void Entropic::Flux::DesktopPlatform::UpdateGUI() {
    ImGui::Begin("Delay Pedal Simulator");

    // Potentiometers
    ImGui::SliderFloat("Time", &potentiometer_values[0], 0.0f, 1.0f);
    ImGui::SliderFloat("Feedback", &potentiometer_values[1], 0.0f, 1.0f);
    ImGui::SliderFloat("Mix", &potentiometer_values[2], 0.0f, 1.0f);

    // Switches
    ImGui::Checkbox("Switch 1", &switch_states[0]);
    ImGui::Checkbox("Switch 2", &switch_states[1]);

    // Buttons
    if (ImGui::Button("Bypass")) button_states[0] = true;
    if (ImGui::Button("Tap Tempo")) button_states[1] = true;

    // LEDs
    ImVec4 led_color = led_states[0] ? ImVec4(0,1,0,1) : ImVec4(0.2f,0.2f,0.2f,1);
    ImGui::ColorButton("Bypass LED", led_color);

    ImGui::End();
}

#endif //DESKTOPPLATFORM_H
