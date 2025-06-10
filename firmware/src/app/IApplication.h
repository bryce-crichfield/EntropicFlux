//
// Created by BC118580 on 6/9/2025.
//

#ifndef IAPPLICATION_H
#define IAPPLICATION_H

#include "platform/IPlatform.h"

class IApplication {
public:
    virtual ~IApplication() = default;

    virtual void initialize(IPlatform* platform) = 0;
    virtual void run() = 0;
    virtual void update() = 0;

    // Audio callback - called by platform
    virtual void ProcessAudio(float* input_left, float* input_right,
                             float* output_left, float* output_right,
                             size_t size) = 0;
};

#endif //IAPPLICATION_H
