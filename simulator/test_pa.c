#include <stdio.h>
#include <math.h>
#include <portaudio.h>
#include <termios.h>
#include <unistd.h>
#include <stdatomic.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "effects/effects.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 32
#define FLOAT_TO_FIXED(x) ((int32_t)(x * 1000000))
#define FIXED_TO_FLOAT(x) ((float)x / 1000000.0f)

typedef struct
{
    delay_t     predelay;
    biquad_t    tone;
    fdn_t       fdn;
    schroeder_t schroeder;
    float mix;
} model_t;
static pthread_mutex_t model_mutex = PTHREAD_MUTEX_INITIALIZER;

void model_init(model_t *model)
{
    biquad_init(&model->tone, FILTER_LOWPASS, 1000, 0.707f, 2, SAMPLE_RATE);
    delay_init(&model->predelay, SAMPLE_RATE);
    delay_set_delay_time(&model->predelay, 0.0f, SAMPLE_RATE);

    fdn_init(&model->fdn, SAMPLE_RATE);
    schroeder_init(&model->schroeder, SAMPLE_RATE);

}

void model_process(model_t *model, float *in, float *out, size_t buffer_size)
{
    for(unsigned int i = 0; i < buffer_size; i++)
    {
        pthread_mutex_lock(&model_mutex);


        float input  = delay_process(&model->predelay, *in++, 0);
        float output = input;


        // output = schroeder_process(&model->schroeder, output);
        // float shimmer = harmonizer_process(&model->harmonizer, output);
        // output = (output + shimmer) / 2;
        output = fdn_process(&model->fdn, output);

        output = biquad_process(&model->tone, output);

        output = tanhf(output);
        *out++ = (1.0f - model->mix) * output + model->mix * input;

        pthread_mutex_unlock(&model_mutex);
    }
}

static model_t model;

typedef struct
{
    const char *name;
    float       min;
    float       max;
    float       step;
    float       default_value;
    atomic_int  atomic_value;
} parameter_t;

static parameter_t parameters[]
    = {{.name          = "Decay",
        .min           = 0.0f,
        .max           = 10.0f,
        .step          = 0.01f,
        .default_value = 2.0f,
        .atomic_value  = ATOMIC_VAR_INIT(FLOAT_TO_FIXED(2.0f))},
       {.name          = "Mix",
        .min           = 0.0f,
        .max           = 1.0f,
        .step          = 0.01f,
        .default_value = 10.0f,
        .atomic_value  = ATOMIC_VAR_INIT(FLOAT_TO_FIXED(10.0f))},
       {.name          = "Damping",
        .min           = 0.0f,
        .max           = 1.0f,
        .step          = 0.01f,
        .default_value = 9.0f,
        .atomic_value  = ATOMIC_VAR_INIT(FLOAT_TO_FIXED(9.0f))},
       {.name          = "Predelay",
        .min           = 0.0f,
        .max           = 1.0f,
        .step          = 0.01f,
        .default_value = 8.0f,
        .atomic_value  = ATOMIC_VAR_INIT(FLOAT_TO_FIXED(8.0f))},
       {.name          = "Tone Control",
        .min           = 0.0f,
        .max           = 1.0f,
        .step          = 0.01f,
        .default_value = 11.0f,
        .atomic_value  = ATOMIC_VAR_INIT(FLOAT_TO_FIXED(11.0f))},
        {.name          = "Modulation Rate",
         .min           = 0.0f,
         .max           = 1.0f,
         .step          = 0.01f,
         .default_value = 1.0f,
         .atomic_value  = ATOMIC_VAR_INIT(FLOAT_TO_FIXED(1.0f))},
};

typedef struct
{
    const char *name;
    void (*update_fn)(void *, float);
    void *model;
} parameter_callback_t;

void parameter_callback_decay(void *model, float value)
{
    model_t *m = (model_t *)model;
    fdn_set_decay(&m->fdn, value);
    schroeder_set_decay(&m->schroeder, value);
}

void parameter_callback_mix(void *model, float value)
{
    model_t *m = (model_t *)model;
    m->mix    = value;
}

void parameter_callback_damping(void *model, float value)
{
    model_t *m = (model_t *)model;
    // fdn_set_damping(&m->fdn, value);
    // schroeder_set_damping(&m->schroeder, value);
}

void parameter_callback_predelay(void *model, float value)
{
    model_t *m = (model_t *)model;
    delay_set_delay_time(&m->predelay, value, SAMPLE_RATE);
}

void parameter_callback_tone_control(void *model, float value)
{
    model_t *m         = (model_t *)model;
    float    log_min   = logf(100.0f);
    float    log_max   = logf(22050.0f);
    float    log_value = log_min + (value * (log_max - log_min));
    float    freq      = expf(log_value);
    biquad_set_frequency(&m->tone, freq);
}

void parameter_callback_modulation_rate(void *model, float value)
{
    model_t *m = (model_t *)model;
    fdn_set_modulation_rate(&m->fdn, value);
    schroder_set_modulation_rate(&m->schroeder, value);
}


static parameter_callback_t parameter_callbacks[] = {
    {.name = "Decay", .update_fn = parameter_callback_decay, .model = &model},
    {.name = "Mix", .update_fn = parameter_callback_mix, .model = &model},
    {.name      = "Damping",
     .update_fn = parameter_callback_damping,
     .model     = &model},
    {.name      = "Predelay",
     .update_fn = parameter_callback_predelay,
     .model     = &model},
    {.name      = "Tone Control",
     .update_fn = parameter_callback_tone_control,
     .model     = &model},
    {.name      = "Modulation Rate",
     .update_fn = parameter_callback_modulation_rate,
     .model     = &model},
};

// ========================================================================================================

typedef struct
{
    double               avg_callback_time;
    uint64_t             total_callbacks;
    uint64_t             underflows;
    double               max_callback_time;
    double               min_callback_time;
    atomic_uint_fast64_t callback_count;
    atomic_uint_fast64_t underflow_count;
    atomic_uint_fast64_t total_process_time_us;
} performance_metrics_t;

static performance_metrics_t metrics
    = {.avg_callback_time     = 0.0,
       .total_callbacks       = 0,
       .underflows            = 0,
       .max_callback_time     = 0.0,
       .min_callback_time     = DBL_MAX,
       .callback_count        = ATOMIC_VAR_INIT(0),
       .underflow_count       = ATOMIC_VAR_INIT(0),
       .total_process_time_us = ATOMIC_VAR_INIT(0)};

static gboolean update_performance_metrics(gpointer user_data)
{
    uint64_t callbacks  = atomic_load(&metrics.callback_count);
    uint64_t underflows = atomic_load(&metrics.underflow_count);
    uint64_t total_time = atomic_load(&metrics.total_process_time_us);

    if(callbacks > 0)
    {
        double avg_time = (double)total_time / callbacks;

        // Update labels with the metrics
        char buffer[256];
        snprintf(buffer,
                 sizeof(buffer),
                 "Avg: %.2f us\nUnderflows: %lu\nTotal Callbacks: %lu",
                 avg_time,
                 underflows,
                 callbacks);

        GtkLabel *metrics_label = GTK_LABEL(user_data);
        gtk_label_set_text(metrics_label, buffer);
    }

    return G_SOURCE_CONTINUE;
}
static void on_value_changed(GtkRange *range)
{
    const char *name  = gtk_widget_get_name(GTK_WIDGET(range));
    float       value = (float)gtk_range_get_value(range);
    int         fixed = FLOAT_TO_FIXED(value);

    for(int i = 0; i < sizeof(parameters) / sizeof(parameters[0]); i++)
    {
        if(strcmp(parameters[i].name, name) == 0)
        {
            atomic_store(&parameters[i].atomic_value, fixed);
            break;
        }
    }

    pthread_mutex_lock(&model_mutex);

    for(int i = 0;
        i < sizeof(parameter_callbacks) / sizeof(parameter_callbacks[0]);
        i++)
    {
        parameter_callback_t *callback = &parameter_callbacks[i];
        if(strcmp(callback->name, name) == 0)
        {
            callback->update_fn(&model, value);
            break;
        }
    }

    pthread_mutex_unlock(&model_mutex);
}

static void update_value_label(GtkLabel *label, GtkRange *range, void *ptr)
{
    char value_text[32];
    snprintf(
        value_text, sizeof(value_text), "%.3f", gtk_range_get_value(range));
    gtk_label_set_text(label, value_text);
}

static void add_labeled_scale(GtkWidget  *grid,
                              const char *label_text,
                              const char *widget_name,
                              double      min,
                              double      max,
                              double      step,
                              int         row)
{
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *scale
        = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, min, max, step);
    GtkWidget *value_label = gtk_label_new("0.00");

    gtk_widget_set_name(scale, widget_name);
    gtk_widget_set_hexpand(scale, TRUE);

    g_signal_connect(
        scale, "value-changed", G_CALLBACK(on_value_changed), NULL);
    g_signal_connect_swapped(
        scale, "value-changed", G_CALLBACK(update_value_label), value_label);

    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scale, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_label, 2, row, 1, 1);
}

typedef struct
{
    GtkWidget *window;
    GtkWidget *grid;
} gtk_t;

static gtk_t gui;

static GtkWidget *gui_init(GtkApplication *app)
{
    gui.window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(gui.window), 1200, 600);

    // Create main horizontal box to hold all columns
    GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(main_hbox, 10);
    gtk_widget_set_margin_end(main_hbox, 10);
    gtk_widget_set_margin_top(main_hbox, 10);
    gtk_widget_set_margin_bottom(main_hbox, 10);

    // Room parameters frame
    GtkWidget *room_frame = gtk_frame_new("Reverb Parameters");
    GtkWidget *room_grid  = gtk_grid_new();
    gtk_widget_set_margin_start(room_grid, 10);
    gtk_widget_set_margin_end(room_grid, 10);
    gtk_widget_set_margin_top(room_grid, 10);
    gtk_widget_set_margin_bottom(room_grid, 10);
    gtk_frame_set_child(GTK_FRAME(room_frame), room_grid);

    // Create all parameter widgets
    for(int i = 0; i < sizeof(parameters) / sizeof(parameters[0]); i++)
    {
        add_labeled_scale(room_grid,
                          parameters[i].name,
                          parameters[i].name,
                          parameters[i].min,
                          parameters[i].max,
                          parameters[i].step,
                          i);
    }

    // Make each frame expand equally
    gtk_widget_set_hexpand(room_frame, TRUE);

    // Add performance metrics frame
    GtkWidget *metrics_frame = gtk_frame_new("Performance Metrics");
    GtkWidget *metrics_label = gtk_label_new("Initializing...");
    gtk_frame_set_child(GTK_FRAME(metrics_frame), metrics_label);
    gtk_box_append(GTK_BOX(main_hbox), metrics_frame);

    // Add timer to update metrics
    g_timeout_add(1000, update_performance_metrics, metrics_label);

    // Add all frames to the main horizontal box
    gtk_box_append(GTK_BOX(main_hbox), room_frame);

    // Set the main box as the window's child
    gtk_window_set_child(GTK_WINDOW(gui.window), main_hbox);
    gtk_window_present(GTK_WINDOW(gui.window));

    return gui.window;
}


static int pa_callback(const void                     *inputBuffer,
                       void                           *outputBuffer,
                       unsigned long                   framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags           statusFlags,
                       void                           *userData)
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Check for underflow
    if(statusFlags & paOutputUnderflow)
    {
        atomic_fetch_add(&metrics.underflow_count, 1);
    }

    float *in  = (float *)inputBuffer;
    float *out = (float *)outputBuffer;
    model_process(&model, in, out, framesPerBuffer);

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate callback duration in microseconds
    double duration_us = (end.tv_sec - start.tv_sec) * 1e6
                         + (end.tv_nsec - start.tv_nsec) / 1e3;

    atomic_fetch_add(&metrics.callback_count, 1);
    atomic_fetch_add(&metrics.total_process_time_us, duration_us);

    return paContinue;
}


static void activate(GtkApplication *app, gpointer user_data)
{
    gui_init(app);
    model_init(&model);

    PaStream *stream;
    PaError   err = Pa_OpenDefaultStream(&stream,
                                       1,
                                       1,
                                       paFloat32,
                                       SAMPLE_RATE,
                                       FRAMES_PER_BUFFER,
                                       pa_callback,
                                       NULL);
    if(err != paNoError)
    {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return;
    }

    err = Pa_StartStream(stream);
    if(err != paNoError)
    {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return;
    }
}

int main(void)
{
    Pa_Initialize();

    // Reset performance metrics
    memset(&metrics, 0, sizeof(metrics));
    metrics.min_callback_time = DBL_MAX;

    GtkApplication *app
        = gtk_application_new("org.example.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 0, NULL);

    Pa_Terminate();
    return status;
}
