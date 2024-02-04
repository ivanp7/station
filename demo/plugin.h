#pragma once

#include <station/plugin.def.h>
#include <station/fsm.def.h>
#include <station/signal.def.h>
#include <station/concurrent.def.h>
#include <station/concurrent.typ.h>
#include <station/sdl.typ.h>
#include <station/font.typ.h>

#include <stdbool.h>
#include <threads.h>

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#endif


// Parameters for concurrent execution of pfunc_inc() and pfunc_dec()
#define NUM_TASKS 1024
#define BATCH_SIZE 16 // number of tasks each thread does at once
#define NUM_ITERATIONS (1024)

#define QUEUE_ALIGNMENT_LOG2 4 // log2 of lock-free queue element alignment
#define QUEUE_CAPACITY_LOG2 2 // log2 of lock-free queue capacity

#define ALARM_DELAY 5 // argument for alarm()

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 144
#define WINDOW_SCALE 4 // window pixels per texture pixel


struct station_signal_set;
struct station_state;


// Plugin's own resources
struct plugin_resources {
    struct station_std_signal_set *std_signals; // standard signals flags
    struct station_rt_signal_set *rt_signals;   // real-time signals flags

    station_concurrent_processing_context_t
        *concurrent_processing_context; // for multithreaded rendering

#ifdef STATION_IS_SDL_SUPPORTED
    SDL_Event event; // for window events
#endif
    station_sdl_window_context_t sdl_window; // window context
    bool sdl_window_created; // whether window was created
    bool window_frozen; // whether window texture is updated

    station_font_psf2_t *font;
    const char *text; // floating text

    // test counter with its mutex
    int counter;
    mtx_t counter_mutex;

    // test lock-free queue
    struct station_queue *queue;

    // for FPS computation
    bool alarm_set;
    unsigned prev_frame, frame;
};


// Signal handler function
static STATION_SIGNAL_HANDLER_FUNC(signal_handler);

// Concurrent processing functions
static STATION_PFUNC_CALLBACK(pfunc_cb_flag);

static STATION_PFUNC(pfunc_inc);
static STATION_PFUNC(pfunc_dec);

static STATION_PFUNC(pfunc_queue);

#ifdef STATION_IS_SDL_SUPPORTED
static STATION_PFUNC(pfunc_draw);
#endif

// State functions for the finite state machine
static STATION_SFUNC(sfunc_pre);

static STATION_SFUNC(sfunc_loop);

#ifdef STATION_IS_SDL_SUPPORTED
static STATION_SFUNC(sfunc_loop_sdl);
#endif

