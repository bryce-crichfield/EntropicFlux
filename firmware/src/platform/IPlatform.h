//
// Created by BC118580 on 6/9/2025.
//

#ifndef IPLATFORM_H
#define IPLATFORM_H
typedef void (*AudioCallback)(float* input_left, float* input_right,
                                    float* output_left, float* output_right,
                                    size_t size);

class IPlatform {
public:
    virtual ~IPlatform() = default;

    // Audio
    virtual void setAudioCallback(AudioCallback callback) = 0;
    virtual void startAudio() = 0;
    virtual void stopAudio() = 0;
    virtual float getSampleRate() const = 0;
    virtual size_t getBlockSize() const = 0;

    // Controls (6 pots, 4 switches, 2 buttons, 2 LEDs based on your code)
    virtual float getPotentiometer(int index) = 0;  // 0-5, returns 0.0-1.0
    virtual bool getSwitch(int index) = 0;          // 0-3
    virtual bool getButton(int index) = 0;          // 0-1, returns true on press
    virtual void setLed(int index, bool state) = 0; // 0-1

    // System
    virtual void delayMs(uint32_t ms) = 0;
    virtual uint32_t getTimeMs() = 0;
    virtual void log(const char* message) = 0;

    // Update loop (called from main)
    virtual void update() = 0;

    // Memory management
    virtual void* allocate(size_t size) = 0;
    virtual void deallocate(void* ptr) = 0;
    virtual void clear(void* ptr, size_t size) = 0;
};
#endif //IPLATFORM_H
