#pragma once

#include <stdint.h>
#include <stddef.h>

template <typename T>
class Delay {
   public:
    Delay() : line_(nullptr), max_size_(0) {}
    ~Delay() {
        if (line_) {
            deallocate(line_);
        }
    }

    bool Init(size_t size) {
        line_ = static_cast<T*>(allocate(size * sizeof(T)));
        if (!line_) {
            return false;
        }
        max_size_ = size;
        Reset();
        return true;
    }

    void Reset() {
        zero_memory(line_, max_size_ * sizeof(T));
        write_ptr_ = 0;
        delay_ = 1;
        frac_ = 0.0f;
    }

    inline void SetDelay(size_t delay) {
        frac_ = 0.0f;
        delay_ = delay < max_size_ ? delay : max_size_ - 1;
    }

    inline void SetDelay(float delay) {
        int32_t int_delay = static_cast<int32_t>(delay);
        frac_ = delay - static_cast<float>(int_delay);
        delay_ = static_cast<size_t>(int_delay) < max_size_ ? int_delay : max_size_ - 1;
    }

    inline void Write(const T sample) {
        line_[write_ptr_] = sample;
        write_ptr_ = (write_ptr_ - 1 + max_size_) % max_size_;
    }

    inline const T Read() const {
        T a = line_[(write_ptr_ + delay_) % max_size_];
        T b = line_[(write_ptr_ + delay_ + 1) % max_size_];
        return a + (b - a) * frac_;
    }

   private:
    float frac_;
    size_t write_ptr_;
    size_t delay_;
    size_t max_size_;
    T* line_;
};