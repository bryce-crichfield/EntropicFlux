#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef DAISY
    #define PLATFORM_DAISY
    #include "daisy_seed.h"
#else
    #define PLATFORM_PC
    #include <stdlib.h> 
    #include <string.h>
#endif

// Define a large static buffer in SDRAM for Daisy platform
#ifdef PLATFORM_DAISY
    #define SDRAM_BUFFER_SIZE (64 * 1024 * 1024)  // 64MB
    static float DSY_SDRAM_BSS sdram_buffer[SDRAM_BUFFER_SIZE / sizeof(float)];
    static size_t sdram_offset = 0;
#endif

static inline void* allocate(size_t size)
{
    #ifdef PLATFORM_DAISY
        // Ensure 4-byte alignment
        size = (size + 3) & ~3;
        
        // Check if we have enough space
        if (sdram_offset + size > SDRAM_BUFFER_SIZE) {
            return NULL;
        }
        
        void* ptr = &sdram_buffer[sdram_offset / sizeof(float)];
        sdram_offset += size;
        return ptr;
    #else
        return malloc(size);
    #endif
}

static inline void deallocate(void* ptr) {
    #ifdef PLATFORM_DAISY
        // No-op for static allocation
    #else
        free(ptr);
    #endif
}

static inline void zero_memory(void* ptr, size_t size) {
    #ifdef PLATFORM_DAISY
        float* fptr = (float*)ptr;
        for(size_t i = 0; i < size/sizeof(float); i++) {
            fptr[i] = 0.0f;
        }
    #else
        memset(ptr, 0, size);
    #endif
}