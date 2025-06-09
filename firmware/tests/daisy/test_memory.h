#include "daisy_seed.h"

#define DAISY

#include "platform/memory.h"


using namespace daisy;

DaisySeed hw;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    // Audio callback to keep system running
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = out[1][i] = 0.0f;
    }
}

int main(void)
{
    hw.Configure();
    hw.Init();
    hw.StartAudio(AudioCallback);
    
    hw.StartLog(true);
    
    hw.PrintLine("Testing allocator...");
    
    // Allocate and print addresses first
    int* buffer1 = (int*)allocate(1024 * sizeof(int));
    int* buffer2 = (int*)allocate(2048 * sizeof(int));
    
    hw.PrintLine("Buffer1 address: 0x%08X", (uint32_t)buffer1);
    hw.PrintLine("Buffer2 address: 0x%08X", (uint32_t)buffer2);

    if(buffer1 && buffer2)
    {
        // Write integers instead of floats
        for(int i = 0; i < 1024; i++)
            buffer1[i] = i;
        
        for(int i = 0; i < 2048; i++)
            buffer2[i] = i * 2;

        // Print first 10 values using integer format
        for(int i = 0; i < 10; i++)
        {
            hw.PrintLine("Buffer1[%d] = %d", i, buffer1[i]);
            hw.PrintLine("Buffer2[%d] = %d", i, buffer2[i]);
        }
    }
    else
    {
        hw.PrintLine("Allocation failed!");
    }

    while(1) {}
}