#pragma once

#include "memory.h"

class CircularBuffer {
    float *buffer;
    size_t size;
    size_t writePosition;
    size_t readPosition;

   public:
    CircularBuffer(size_t bufferSize) {
        buffer = static_cast<float *>(allocate(bufferSize * sizeof(float)));
        zero_memory(buffer, bufferSize * sizeof(float));
        size = bufferSize;
        writePosition = 0;
        readPosition = 0;
    }

    ~CircularBuffer() {
        if (buffer) {
            free(buffer);
        }
    }

    void fill(float value) {
        for (size_t i = 0; i < size; i++) {
            buffer[i] = value;
        }
    }

    void write(const float value) {
        buffer[writePosition] = value;
        writePosition = (writePosition + 1) % size;
    }

    void write(const float value, const size_t position) const {
        buffer[position % size] = value;
    }

    float read(size_t position) {
        if (!buffer || position >= size)
            return 0.0f;
        return buffer[position % size];
    }

    float interpolate(float position) {
        size_t pos1 = (size_t)position;
        size_t pos2 = (pos1 + 1) % size;
        float frac = position - (float)pos1;
        float s1 = read(pos1);
        float s2 = read(pos2);
        return s1 * (1.0f - frac) + s2 * frac;
    }

    size_t getSize() const {
        return size;
    }
    size_t getWritePos() const {
        return writePosition;
    }
    size_t getReadPos() const {
        return readPosition;
    }
};