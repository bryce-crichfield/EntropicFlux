#pragma once

static float clamp(float val, float min, float max)
{
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

/**
 * @brief Scales the input array and stores the result in the output array.
 *
 * This function scales the values in the input array based on the provided
 * magnitude and variance scales, and ensures that the output values do not
 * exceed the specified maximum value.
 *
 * @param input            Pointer to the input array.
 * @param output           Pointer to the output array where the scaled values will be stored.
 * @param length           The number of elements in the input and output arrays.
 * @param magnitude_scale  The factor by which the mean of the input array is scaled.
 * @param variance_scale   The factor by which the variance from the mean is scaled.
 * @param max              The maximum allowable value for the output array elements.
 */
void scale(float *input,
           float *output,
           int    length,
           float  magnitude_scale,
           float  variance_scale,
           float  max)
{
    // Calculate mean
    float mean = 0;
    for(int i = 0; i < length; i++)
    {
        mean += input[i];
    }
    mean /= length;

    // Scale to output array
    for(int i = 0; i < length; i++)
    {
        float distance_from_mean = input[i] - mean;
        float scaled_distance    = distance_from_mean * variance_scale;
        output[i]                = (mean + scaled_distance) * magnitude_scale;
        if(output[i] > max)
        {
            output[i] = max;
        }
    }
}


float map_range(float value, float in_min, float in_max, float out_min, float out_max) 
{
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}