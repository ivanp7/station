#include <station/plugin.h>
#include <station/func.def.h>
#include <station/signal.typ.h>

#include <stdbool.h>
#include <threads.h>

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#endif


#define NUM_TASKS 128
#define BATCH_SIZE 16

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 144
#define WINDOW_SCALE 4

#define ALARM_DELAY 5


struct plugin_resources {
    station_signal_set_t *signals;

#ifdef STATION_IS_SDL_SUPPORTED
    SDL_Event event;
#endif
    station_sdl_context_t *sdl_context;
    station_opencl_context_t *opencl_context;

    int counter;
    mtx_t counter_mutex;

    bool frozen;
    bool alarm_set;

    unsigned prev_frame, frame;
};


static STATION_PFUNC(pfunc_inc);
static STATION_PFUNC(pfunc_dec);
static STATION_PFUNC(pfunc_draw);

static STATION_SFUNC(sfunc_pre);
static STATION_SFUNC(sfunc_loop);
static STATION_SFUNC(sfunc_post);


STATION_PLUGIN_PREAMBLE()

