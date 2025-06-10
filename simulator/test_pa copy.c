#include <stdio.h>
#include <math.h>
#include <portaudio.h>
#include <termios.h>
#include <unistd.h>

#include "effects/effects.h"
#include "gui/gui.h"
#include "gui/oscilloscope.h"
#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 64

static oscillator_t  osc;
static oscillator_t  lfo;
static arpeggiator_t arp;
static harmonizer_t  hm;
static reverb_t      rv;
static adsr_t        adsr;
static biquad_t      bq;
static delay_t       dl;
static noise_t       noise;
static follower_t    follower;
static granulator_t  grn;

static GUI     gui;
static scope_t scope;

static int pa_callback(const void                     *inputBuffer,
                      void                           *outputBuffer,
                      unsigned long                   framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags           statusFlags,
                      void                           *userData)
{
    float *in  = (float *)inputBuffer;
    float *out = (float *)outputBuffer;

    for(unsigned int i = 0; i < framesPerBuffer; i++)
    {
        // float freq   = arpeggiator_process(&arp);
        // if(freq > 0.0f)
        // {
        //     oscillator_set_frequency(&osc, freq);
        //     if(adsr.current_stage == ADSR_IDLE)
        //     {
        //         adsr_gate_on(&adsr);
        //     }
        // }
        // else if(adsr.current_stage != ADSR_IDLE
        //         && adsr.current_stage != ADSR_RELEASE)
        // {
        //     adsr_gate_off(&adsr);
        // }
        float output = *in++;

        // float envelope = adsr_process(&adsr);
        // output = oscillator_process(&osc);
        // float lfo_val   = oscillator_process(&lfo);
        // float noise_val = noise_process(&noise);
        // move the harmonizer frequency up and down with the LFO
        // harmonizer_modulate(&hm, noise_val * 0.25f);
        // biquad_set_frequency(&bq, 1000.0f + (noise_val * 500.0f));


        // float env = follower_process(&follower, output);
        // harmonizer_modulate(&hm, env * 2.5f);
        // output       = harmonizer_process(&hm, output);

        // output = granulator_process(&grn, output);

        // float feedback = 0.5f * (noise_val * 0.5f * 0.1f);
        // output += delay_process(&dl, output, feedback);
        // output = biquad_process(&bq, output);
        output = reverb_process(&rv, output);

        output = softclip_process(output * 0.75f);
        *out++ = output;

        scope_process(&scope, output);
    }

    return paContinue;
}


int main(void)
{
    PaStream *stream;
    PaError   err;

    err = Pa_Initialize();
    if(err != paNoError)
        goto error;

    oscillator_init(&osc, 44100);
    oscillator_set_waveform(&osc, SINE);
    oscillator_set_frequency(&osc, 440.0f);

    oscillator_init(&lfo, 44100);
    oscillator_set_waveform(&lfo, SINE);
    oscillator_set_frequency(&lfo, 0.1f);

    arpeggiator_init(
        &arp, 44100, 440.0f, 120.0f);          // 440 Hz base frequency, 120 BPM
    arpeggiator_set_pattern(&arp, "0-4-7-4-"); // Set custom pattern

    adsr_init(&adsr, 44100);
    adsr_set_params(&adsr, 0.000001f, 0.1f, 0.000001f, 0.000001f);

    harmonizer_init(&hm, SAMPLE_RATE);
    harmonizer_set_harmony(&hm, PERFECT_FIFTH);

    reverb_init(&rv, SAMPLE_RATE);
    reverb_set_model(&rv, &REVERB_HALL);

    biquad_init(&bq, FILTER_LOWPASS, 10000.0f, 0.7f, 2, SAMPLE_RATE);

    scope_init(&scope, SAMPLE_RATE / 4);

    delay_init(&dl, SAMPLE_RATE);
    delay_set_delay_time(&dl, 0.4f, SAMPLE_RATE);

    noise_init(&noise, SAMPLE_RATE);
    noise_set_frequency(&noise, 100.0f);

    follower_init(&follower, SAMPLE_RATE);

    // Set parameters
    err = Pa_OpenDefaultStream(&stream,
                               1,         // one input channel
                               1,         // mono output
                               paFloat32, // sample format
                               SAMPLE_RATE,
                               FRAMES_PER_BUFFER,
                               pa_callback,
                               NULL);
    if(err != paNoError)
        goto error;

    gui_init(&gui, "PortAudio Test", 800, 600);


    err = Pa_StartStream(stream);
    if(err != paNoError)
        goto error;

    printf("Press ESC to stop the audio...\n");
    while(1)
    {
        if(!gui_handle_events(&gui))
            break;

        scope_render(&gui, &scope);
    }

    err = Pa_StopStream(stream);
    if(err != paNoError)
        goto error;

    Pa_CloseStream(stream);
    Pa_Terminate();
    return 0;

error:
    Pa_Terminate();
    fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
    return 1;
}
