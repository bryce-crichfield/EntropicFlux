#pragma once

#include <math.h>
#include <stdlib.h>

#include "daisy_seed.h"

#define VARIABLE_SMOOTHING 0.995f

enum PotentiometerCurve {
    LINEAR,
    EXPONENTIAL,
    LOGARITHMIC,
};

class Potentiometer {
    float value;
    float old_raw_value;
    float min;
    float max;
    PotentiometerCurve curve;
    size_t index;
    daisy::AdcHandle* adc;
    bool was_updated;

   public:
    Potentiometer() {}

    void Init(daisy::AdcHandle* adc_handle, size_t adc_index, float min_value, float max_value,
              PotentiometerCurve curve_type) {
        adc = adc_handle;
        index = adc_index;
        min = min_value;
        max = max_value;
        curve = curve_type;
        value = min;
    }

    void Update() {
        float raw_value = adc->GetFloat(index);
        float scaled_value;

        // First apply the curve to raw ADC value
        switch (curve) {
            case LINEAR:
                scaled_value = min + (max - min) * raw_value;
                break;
            case EXPONENTIAL:
                // Prevent issues with min=0 and ensure smoother exponential curve
                scaled_value = min + (max - min) * (raw_value * raw_value);
                break;
            case LOGARITHMIC:
                // More natural logarithmic response
                scaled_value = min + (max - min) * (logf(raw_value * 9.0f + 1.0f) / logf(10.0f));
                break;
            default:
                scaled_value = min + (max - min) * raw_value;
                break;
        }

        // Then apply smoothing to the scaled value
        value = (VARIABLE_SMOOTHING * value) + ((1.0f - VARIABLE_SMOOTHING) * scaled_value);

        // Check if the value has changed
        was_updated = fabsf(raw_value - old_raw_value) > 0.001f;
        old_raw_value = raw_value;
    }

    void SetValue(float new_value) {
        value = new_value;
    }

    float GetValue() {
        return value;
    }

    bool WasUpdated() {
        return was_updated;
    }
};

