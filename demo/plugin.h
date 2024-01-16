#include <station/plugin.def.h>
#include <station/fsm.def.h>
#include <station/parallel.def.h>
#include <station/parallel.typ.h>
#include <station/sdl.typ.h>

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


struct station_signal_set;
struct station_state;

struct plugin_resources {
    struct station_signal_set *signals;

    station_parallel_processing_context_t *parallel_processing_context;

#ifdef STATION_IS_SDL_SUPPORTED
    SDL_Event event;
#endif
    station_sdl_window_context_t sdl_window;
    bool sdl_window_created;

    int counter;
    mtx_t counter_mutex;

    bool frozen;
    bool alarm_set;

    unsigned prev_frame, frame;
};


static STATION_PFUNC(pfunc_inc);
static STATION_PFUNC(pfunc_dec);

static STATION_SFUNC(sfunc_pre);
static STATION_SFUNC(sfunc_loop);
static STATION_SFUNC(sfunc_post);

#ifdef STATION_IS_SDL_SUPPORTED
static STATION_PFUNC(pfunc_draw);
static STATION_SFUNC(sfunc_loop_sdl);
#endif

