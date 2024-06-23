/*****************************************************************************
 * Copyright (C) 2020-2024 by Ivan Podmazov                                  *
 *                                                                           *
 * This file is part of Station.                                             *
 *                                                                           *
 *   Station is free software: you can redistribute it and/or modify it      *
 *   under the terms of the GNU Lesser General Public License as published   *
 *   by the Free Software Foundation, either version 3 of the License, or    *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   Station is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU Lesser General Public License for more details.                     *
 *                                                                           *
 *   You should have received a copy of the GNU Lesser General Public        *
 *   License along with Station. If not, see <http://www.gnu.org/licenses/>. *
 *****************************************************************************/

/**
 * @file
 * @brief Application implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__STDC_NO_THREADS__) || defined(__STDC_NO_ATOMICS__)
#  undef STATION_IS_CONCURRENT_PROCESSING_SUPPORTED
#  undef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
#endif

#ifdef STATION_IS_DLFCN_SUPPORTED
#  include <dlfcn.h>
#endif

#ifdef STATION_IS_CONCURRENT_PROCESSING_SUPPORTED
#  include <threads.h>
#  include <stdatomic.h>
#endif

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
#  include <signal.h>
#endif

#ifdef STATION_IS_OPENCL_SUPPORTED
#  include <CL/cl.h>
#endif

#ifdef STATION_IS_SDL_SUPPORTED
#  include <SDL.h>
#endif

#include <station/application.fun.h>
#include <station/application.def.h>

#include <station/plugin.typ.h>
#include <station/plugin.def.h>

#include <station/signal.fun.h>
#include <station/signal.typ.h>

#include <station/concurrent.fun.h>

#include <station/opencl.typ.h>
#include <station/sdl.typ.h>

#include <station/buffer.fun.h>
#include <station/buffer.typ.h>

#include "application.args.h"

/*****************************************************************************/

#ifdef STATION_IS_ANSI_ESCAPE_CODES_ENABLED

#  define COLOR_RESET         "\033[0m"

#  define COLOR_FG_BLACK      "\033[30m"
#  define COLOR_FG_RED        "\033[31m"
#  define COLOR_FG_GREEN      "\033[32m"
#  define COLOR_FG_YELLOW     "\033[33m"
#  define COLOR_FG_BLUE       "\033[34m"
#  define COLOR_FG_MAGENTA    "\033[35m"
#  define COLOR_FG_CYAN       "\033[36m"
#  define COLOR_FG_WHITE      "\033[37m"

#  define COLOR_BG_BLACK      "\033[40m"
#  define COLOR_BG_RED        "\033[41m"
#  define COLOR_BG_GREEN      "\033[42m"
#  define COLOR_BG_YELLOW     "\033[43m"
#  define COLOR_BG_BLUE       "\033[44m"
#  define COLOR_BG_MAGENTA    "\033[45m"
#  define COLOR_BG_CYAN       "\033[46m"
#  define COLOR_BG_WHITE      "\033[47m"

#  define COLOR_FG_BRI_BLACK      "\033[90m"
#  define COLOR_FG_BRI_RED        "\033[91m"
#  define COLOR_FG_BRI_GREEN      "\033[92m"
#  define COLOR_FG_BRI_YELLOW     "\033[93m"
#  define COLOR_FG_BRI_BLUE       "\033[94m"
#  define COLOR_FG_BRI_MAGENTA    "\033[95m"
#  define COLOR_FG_BRI_CYAN       "\033[96m"
#  define COLOR_FG_BRI_WHITE      "\033[97m"

#  define COLOR_BG_BRI_BLACK      "\033[100m"
#  define COLOR_BG_BRI_RED        "\033[101m"
#  define COLOR_BG_BRI_GREEN      "\033[102m"
#  define COLOR_BG_BRI_YELLOW     "\033[103m"
#  define COLOR_BG_BRI_BLUE       "\033[104m"
#  define COLOR_BG_BRI_MAGENTA    "\033[105m"
#  define COLOR_BG_BRI_CYAN       "\033[106m"
#  define COLOR_BG_BRI_WHITE      "\033[107m"

#else

#  define COLOR_RESET         ""

#  define COLOR_FG_BLACK      ""
#  define COLOR_FG_RED        ""
#  define COLOR_FG_GREEN      ""
#  define COLOR_FG_YELLOW     ""
#  define COLOR_FG_BLUE       ""
#  define COLOR_FG_MAGENTA    ""
#  define COLOR_FG_CYAN       ""
#  define COLOR_FG_WHITE      ""

#  define COLOR_BG_BLACK      ""
#  define COLOR_BG_RED        ""
#  define COLOR_BG_GREEN      ""
#  define COLOR_BG_YELLOW     ""
#  define COLOR_BG_BLUE       ""
#  define COLOR_BG_MAGENTA    ""
#  define COLOR_BG_CYAN       ""
#  define COLOR_BG_WHITE      ""

#  define COLOR_FG_BRI_BLACK      ""
#  define COLOR_FG_BRI_RED        ""
#  define COLOR_FG_BRI_GREEN      ""
#  define COLOR_FG_BRI_YELLOW     ""
#  define COLOR_FG_BRI_BLUE       ""
#  define COLOR_FG_BRI_MAGENTA    ""
#  define COLOR_FG_BRI_CYAN       ""
#  define COLOR_FG_BRI_WHITE      ""

#  define COLOR_BG_BRI_BLACK      ""
#  define COLOR_BG_BRI_RED        ""
#  define COLOR_BG_BRI_GREEN      ""
#  define COLOR_BG_BRI_YELLOW     ""
#  define COLOR_BG_BRI_BLUE       ""
#  define COLOR_BG_BRI_MAGENTA    ""
#  define COLOR_BG_BRI_CYAN       ""
#  define COLOR_BG_BRI_WHITE      ""

#endif

#define COLOR_STRING COLOR_FG_BRI_WHITE
#define COLOR_NUMBER COLOR_FG_BRI_YELLOW
#define COLOR_FLAG_ON COLOR_FG_GREEN
#define COLOR_FLAG_OFF COLOR_FG_RED
#define COLOR_SIGNAL COLOR_FG_BRI_MAGENTA
#define COLOR_SDL_SUBSYSTEM COLOR_FG_BRI_CYAN
#define COLOR_VERSION COLOR_FG_BRI_BLUE
#define COLOR_OUTPUT_SEGMENT COLOR_FG_BRI_BLACK
#define COLOR_ERROR COLOR_FG_BRI_RED

#define OUTPUT_SEGMENT_BEGIN_HELP \
    "↓↓↓·································· HELP ·································↓↓↓"
#define OUTPUT_SEGMENT_END_HELP \
    "↑↑↑·································· HELP ·································↑↑↑"
#define OUTPUT_SEGMENT_BEGIN_CONF \
    "↓↓↓····························· CONFIGURATION ·····························↓↓↓"
#define OUTPUT_SEGMENT_END_CONF \
    "↑↑↑····························· CONFIGURATION ·····························↑↑↑"
#define OUTPUT_SEGMENT_BEGIN_INIT \
    "↓↓↓····························· INITIALIZATION ····························↓↓↓"
#define OUTPUT_SEGMENT_END_INIT \
    "↑↑↑····························· INITIALIZATION ····························↑↑↑"
#define OUTPUT_SEGMENT_BEGIN_EXEC \
    "↓↓↓······························· EXECUTION ·······························↓↓↓"
#define OUTPUT_SEGMENT_END_EXEC \
    "↑↑↑······························· EXECUTION ·······························↑↑↑"
#define OUTPUT_SEGMENT_BEGIN_FINAL \
    "↓↓↓······························ FINALIZATION ·····························↓↓↓"
#define OUTPUT_SEGMENT_END_FINAL \
    "↑↑↑······························ FINALIZATION ·····························↑↑↑"
#define OUTPUT_SEGMENT_SEPARATOR \
    "==============================================================================="

#define PRINT(msg) fprintf(stderr, msg)
#define PRINT_(msg, ...) fprintf(stderr, msg, __VA_ARGS__)

#define ERROR(msg) fprintf(stderr, "\n" COLOR_ERROR "Error" COLOR_RESET \
        ": " msg ".\n")
#define ERROR_(msg, ...) fprintf(stderr, "\n" COLOR_ERROR "Error" COLOR_RESET \
        ": " msg ".\n", __VA_ARGS__)

#define AT_EXIT(func) do {                      \
    if (atexit(func) != 0) {                    \
        ERROR("atexit(" #func ") has failed");  \
        func();                                 \
        exit(STATION_APP_ERROR_ATEXIT); } } while (0)

#define AT_QUICK_EXIT(func) do {                        \
    if (at_quick_exit(func) != 0) {                     \
        ERROR("at_quick_exit(" #func ") has failed");   \
        func();                                         \
        exit(STATION_APP_ERROR_ATEXIT); } } while (0)

#ifdef STATION_IS_CONCURRENT_PROCESSING_SUPPORTED
#  define EXIT_ASSERT_MAIN_THREAD() do {                        \
    if (!thrd_equal(thrd_current(), application.main_thread)) { \
        ERROR("calling exit() or quick_exit() not from the main thread is forbidden");  \
        abort(); } } while (0)
#else
#  define EXIT_ASSERT_MAIN_THREAD()
#endif

#define STRINGIFY(obj) STRING_OF(obj)
#define STRING_OF(obj) #obj

/*****************************************************************************/

static struct {
    struct gengetopt_args_info args;
    bool verbose;

    struct {
        int argc;
        char **argv;

        void *handle;
        bool built_in;
        station_plugin_vtable_t *vtable;

        station_plugin_conf_func_args_t configuration;

        void *resources;
    } plugin;

    struct {
        size_t count;
        FILE **streams;
    } file;

    struct {
        size_t count;
        void **handles;
    } library;

    struct {
        station_std_signal_set_t std_set;
        station_rt_signal_set_t rt_set;
        bool management_used;
        struct station_signal_management_context *management_context;
    } signal;

    struct {
        station_concurrent_processing_contexts_array_t contexts;
    } concurrent_processing;

    struct {
        station_opencl_contexts_array_t contexts;

#ifdef STATION_IS_OPENCL_SUPPORTED
        struct {
            cl_uint num_platforms;
            cl_platform_id *platform_list;
            cl_uint *num_devices;
            cl_device_id **device_list;
        } tmp;
#endif
    } opencl;

    struct {
        station_state_t state;
        void *data;
    } fsm;

#ifdef STATION_IS_CONCURRENT_PROCESSING_SUPPORTED
    thrd_t main_thread;
#endif

    bool print_final_message;
    bool end_of_main_reached;
} application;

/*****************************************************************************/

static void initialize(int argc, char *argv[]);

static void check_plugin(void);

static void exit_release_args(void);

#ifdef STATION_IS_DLFCN_SUPPORTED
static void exit_unload_plugin(void);
#endif

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
static void exit_stop_signal_management_thread(void);
#endif

static void exit_destroy_concurrent_processing_contexts(void);

#ifdef STATION_IS_SDL_SUPPORTED
static void exit_quit_sdl(void);
#endif

#ifdef STATION_IS_DLFCN_SUPPORTED
static void exit_close_libraries(void);
#endif

static void exit_close_files(void);

static void exit_end_plugin_help_fn_output(void);
static void exit_end_plugin_conf_fn_output(void);

/*****************************************************************************/

static int run(void);
static int finalize(bool quick);

static void exit_finalize_plugin(void);
static void exit_finalize_plugin_quick(void);

static void exit_end_plugin_init_fn_output(void);
static void exit_end_plugin_exec_fn_output(void);

/*****************************************************************************/

#ifdef STATION_IS_OPENCL_SUPPORTED

static void display_opencl_listing(void);

static void prepare_opencl_tmp_arrays(void);
static void create_opencl_contexts(void);
static void free_opencl_tmp_arrays(void);

static void exit_release_opencl_contexts(void);
static void exit_release_opencl_contexts_quick(void);

#endif

/*****************************************************************************/

static void initialize_static_data(void);

static void exit_finalize_static_data(void);

enum termination_reason {
    TR_MAIN,
    TR_EXIT,
    TR_QUICK_EXIT,
};

static void print_final_message(enum termination_reason reason);

static void exit_print_final_message(void);
static void exit_print_final_message_quick(void);

int
station_app_main(
        int argc,
        char *argv[],

        station_plugin_vtable_t *plugin_vtable)
{
    if (plugin_vtable != NULL)
    {
        application.plugin.vtable = plugin_vtable;
        application.plugin.built_in = true;

        check_plugin();
    }

    AT_EXIT(exit_print_final_message);
    AT_QUICK_EXIT(exit_print_final_message_quick);

    initialize_static_data();

    AT_EXIT(exit_finalize_static_data);

    initialize(argc, argv);

    if (application.verbose)
        application.print_final_message = true;

    int exit_code = run();

    application.end_of_main_reached = true;
    return exit_code;
}

static void initialize_static_data(void)
{
#ifdef STATION_IS_CONCURRENT_PROCESSING_SUPPORTED
    application.main_thread = thrd_current();
#endif

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    application.signal.rt_set.signal_SIGRTMIN = malloc(
            sizeof(*application.signal.rt_set.signal_SIGRTMIN) * (SIGRTMAX-SIGRTMIN+1));
    if (application.signal.rt_set.signal_SIGRTMIN == NULL)
    {
        ERROR("couldn't allocate array of real-time signal flags");
        exit(STATION_APP_ERROR_MALLOC);
    }

    for (int i = 0; i <= SIGRTMAX-SIGRTMIN; i++)
        atomic_init(&application.signal.rt_set.signal_SIGRTMIN[i], false);
#endif
}

static void exit_finalize_static_data(void)
{
    EXIT_ASSERT_MAIN_THREAD();

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    free(application.signal.rt_set.signal_SIGRTMIN);
#endif
}

static void print_final_message(enum termination_reason reason)
{
    PRINT("\nTermination reason: " COLOR_STRING);

    switch (reason)
    {
        case TR_MAIN:
            PRINT("reaching end of main()");
            break;

        case TR_EXIT:
            PRINT("call of exit()");
            break;

        case TR_QUICK_EXIT:
            PRINT("call of quick_exit()");
            break;
    }

    PRINT(COLOR_RESET ".\n");
}

static void exit_print_final_message(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    if (application.print_final_message)
        print_final_message(application.end_of_main_reached ? TR_MAIN : TR_EXIT);
}

static void exit_print_final_message_quick(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    if (application.print_final_message)
        print_final_message(TR_QUICK_EXIT);
}

/*****************************************************************************/

static void initialize(int argc, char *argv[])
{
    //////////////////////////////////////////////////
    // Split application and plugin arguments apart //
    //////////////////////////////////////////////////

    {
        int app_argc = argc;
        application.plugin.argv = argv + argc;

        for (int i = 1; i < argc; i++)
            if (strcmp(argv[i], "--") == 0)
            {
                app_argc = i;
                application.plugin.argc = argc - app_argc;
                application.plugin.argv = argv + app_argc;
                break;
            }

        /////////////////////////////////
        // Parse application arguments //
        /////////////////////////////////

        args_parser_init(&application.args);
        AT_EXIT(exit_release_args);

        if (args_parser(app_argc, argv, &application.args) != 0)
        {
            ERROR("couldn't parse application arguments");
            exit(STATION_APP_ERROR_ARGUMENTS);
        }
    }

    ////////////////////////////////////////////
    // Parse application arguments from files //
    ////////////////////////////////////////////

    {
        struct args_parser_params params;
        args_parser_params_init(&params);

        params.override = 0;
        params.initialize = 0;
        params.check_required = 1;
        params.check_ambiguity = 0;
        params.print_errors = 1;

        for (unsigned i = 0; i < application.args.argfile_given; i++)
        {
            if (args_parser_config_file(application.args.argfile_arg[i],
                        &application.args, &params) != 0)
            {
                ERROR_("couldn't parse arguments from file "
                        COLOR_STRING "%s" COLOR_RESET, application.args.argfile_arg[i]);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }
        }
    }

    //////////////////////
    // Display the logo //
    //////////////////////

    if (application.args.logo_given)
    {
        PRINT(COLOR_RESET COLOR_FG_BRI_WHITE "\n\
                                                                \n\
                          █                                     \n\
          ▐▌        ▐▌    ▀                                     \n\
    ▗▟██▖▐███  ▟██▖▐███  ██   ▟█▙ ▐▙██▖      ▟██▖▐▙█▙ ▐▙█▙      \n\
    ▐▙▄▖▘ ▐▌   ▘▄▟▌ ▐▌    █  ▐▛ ▜▌▐▛ ▐▌      ▘▄▟▌▐▛ ▜▌▐▛ ▜▌     \n\
     ▀▀█▖ ▐▌  ▗█▀▜▌ ▐▌    █  ▐▌ ▐▌▐▌ ▐▌ ██▌ ▗█▀▜▌▐▌ ▐▌▐▌ ▐▌     \n\
    ▐▄▄▟▌ ▐▙▄ ▐▙▄█▌ ▐▙▄ ▗▄█▄▖▝█▄█▘▐▌ ▐▌     ▐▙▄█▌▐█▄█▘▐█▄█▘     \n\
     ▀▀▀   ▀▀  ▀▀▝▘  ▀▀ ▝▀▀▀▘ ▝▀▘ ▝▘ ▝▘      ▀▀▝▘▐▌▀▘ ▐▌▀▘      \n\
                                                 ▐▌   ▐▌        \n\
                                                                \n\
                                                     by Ivan Podmazov\n\
                                                      (C) 2020-2024\n\
\n\n" COLOR_RESET);

        fflush(stderr);
    }

    /////////////////////////////////////////////////////
    // Display application version and feature support //
    /////////////////////////////////////////////////////

    application.verbose = application.args.verbose_given;

    if (application.verbose)
    {
        {
            uint32_t version_year = STATION_PLUGIN_VERSION / 10000;
            uint32_t version_month = (STATION_PLUGIN_VERSION / 100) % 100;
            uint32_t version_day = STATION_PLUGIN_VERSION % 100;
            PRINT_("Version: " COLOR_VERSION "%u.%.2u.%.2u" COLOR_RESET "\n\n",
                    version_year, version_month, version_day);
        }

        PRINT("[");
#ifdef STATION_IS_DLFCN_SUPPORTED
        PRINT(COLOR_FLAG_ON "  supported  ");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "] Shared libraries\n");

        PRINT("[");
#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
        PRINT(COLOR_FLAG_ON "  supported  ");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "] Signal management\n");

        PRINT("[");
#ifdef STATION_IS_CONCURRENT_PROCESSING_SUPPORTED
        PRINT(COLOR_FLAG_ON "  supported  ");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "] Concurrent processing\n");

        PRINT("[");
#ifdef STATION_IS_OPENCL_SUPPORTED
        PRINT(COLOR_FLAG_ON "  supported  ");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "] OpenCL\n");

        PRINT("[");
#ifdef STATION_IS_SDL_SUPPORTED
        PRINT(COLOR_FLAG_ON "  supported  ");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "] SDL\n");

        PRINT("\n");
    }

    ////////////////////////////////////////////////
    // Check correctness of application arguments //
    ////////////////////////////////////////////////

    if (application.plugin.built_in && (application.args.inputs_num > 0))
    {
        ERROR("a application with built-in plugin doesn't accept plugin file argument");
        exit(STATION_APP_ERROR_ARGUMENTS);
    }

    if (application.args.cl_list_given)
    {
#ifdef STATION_IS_OPENCL_SUPPORTED
        if (application.args.inputs_num > 0)
        {
            ERROR("processing a plugin file after displaying list of OpenCL platforms/devices is not supported");
            exit(STATION_APP_ERROR_ARGUMENTS);
        }

        if (application.args.help_given)
        {
            ERROR("OpenCL listing mode is mutually exclusive with help mode");
            exit(STATION_APP_ERROR_ARGUMENTS);
        }
#else
        ERROR("OpenCL is not supported");
        exit(STATION_APP_ERROR_ARGUMENTS);
#endif
    }
    else if (application.args.help_given)
    {
        if (application.args.inputs_num > 1)
        {
            ERROR("cannot process more than one plugin file");
            exit(STATION_APP_ERROR_ARGUMENTS);
        }
    }
    else
    {
        if (application.args.inputs_num > 1)
        {
            ERROR("cannot process more than one plugin file");
            exit(STATION_APP_ERROR_ARGUMENTS);
        }

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
        for (unsigned i = 0; i < application.args.SIGRTMIN_given; i++)
        {
            int signal = application.args.SIGRTMIN_arg[i];

            if (signal < 0)
            {
                ERROR_("real-time signal number can't be less than SIGRTMIN (argument ["
                        COLOR_NUMBER "%u" COLOR_RESET "])", i);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }
            else if (signal + SIGRTMIN > SIGRTMAX)
            {
                ERROR_("real-time signal number SIGRTMIN%+i (argument ["
                        COLOR_NUMBER "%u" COLOR_RESET "]) is greater than SIGRTMAX", signal, i);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }
        }

        for (unsigned i = 0; i < application.args.SIGRTMAX_given; i++)
        {
            int signal = application.args.SIGRTMAX_arg[i];

            if (signal > 0)
            {
                ERROR_("real-time signal number can't be greater than SIGRTMAX (argument ["
                        COLOR_NUMBER "%u" COLOR_RESET "])", i);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }
            else if (signal + SIGRTMAX < SIGRTMIN)
            {
                ERROR_("real-time signal number SIGRTMAX%+i (argument ["
                        COLOR_NUMBER "%u" COLOR_RESET "]) is less than SIGRTMIN", signal, i);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }
        }
#endif

#ifdef STATION_IS_OPENCL_SUPPORTED
        for (unsigned i = 0; i < application.args.cl_context_given; i++)
        {
            const char *separator = strchr(application.args.cl_context_arg[i], ':');
            const char *device_mask = NULL;

            if (separator != NULL)
                device_mask = separator + 1;
            else
                separator = application.args.cl_context_arg[i] +
                    strlen(application.args.cl_context_arg[i]);

            if (separator == application.args.cl_context_arg[i])
            {
                ERROR_("OpenCL platform index is empty (context argument ["
                        COLOR_NUMBER "%u" COLOR_RESET "])", i);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }

            char *platform_idx_end;
            strtoul(application.args.cl_context_arg[i], &platform_idx_end, 16);

            if (platform_idx_end != separator)
            {
                ERROR_("OpenCL platform index contains invalid characters: "
                        COLOR_NUMBER "%.*s" COLOR_RESET " (context argument ["
                        COLOR_NUMBER "%u" COLOR_RESET "])",
                        (int)(separator - application.args.cl_context_arg[i]),
                        application.args.cl_context_arg[i], i);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }

            if (device_mask != NULL)
            {
                size_t device_mask_len = strlen(device_mask);

                if (device_mask_len == 0)
                {
                    ERROR_("OpenCL device mask cannot be empty (context argument ["
                            COLOR_NUMBER "%u" COLOR_RESET "])", i);
                    exit(STATION_APP_ERROR_ARGUMENTS);
                }
                else if (strspn(device_mask, "0123456789ABCDEFabcdef") != device_mask_len)
                {
                    ERROR_("OpenCL device mask contains invalid characters: "
                            COLOR_NUMBER "%s" COLOR_RESET " (context argument ["
                            COLOR_NUMBER "%u" COLOR_RESET "])", device_mask, i);
                    exit(STATION_APP_ERROR_ARGUMENTS);
                }
                else if (strspn(device_mask, "0") == device_mask_len)
                {
                    ERROR_("OpenCL device mask cannot be zero (context argument ["
                            COLOR_NUMBER "%u" COLOR_RESET "])", i);
                    exit(STATION_APP_ERROR_ARGUMENTS);
                }
            }
        }
#endif
    }

    ////////////////////////////////////
    // Display application usage help //
    ////////////////////////////////////

    if (application.args.help_given)
    {
        if (!application.plugin.built_in)
        {
            if (application.args.inputs_num == 0)
            {
                PRINT("\nstation-app [options...] [PLUGIN_FILE [-- [plugin options...]]]\n\n");
                args_parser_print_help();
                exit(EXIT_SUCCESS);
            }
        }
        else
        {
            PRINT_("\n%s [app options...] [-- [options...]]\n\n",
                    application.plugin.vtable->info.name);
            args_parser_print_help();
            PRINT("\n");
        }
    }

    //////////////////////////////////////////////
    // Display list of OpenCL platforms/devices //
    //////////////////////////////////////////////

    if (application.args.cl_list_given)
    {
#ifdef STATION_IS_OPENCL_SUPPORTED
        display_opencl_listing();
#endif
        exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////
    // Exit if no plugin file is provided //
    ////////////////////////////////////////

    if (!application.plugin.built_in && (application.args.inputs_num == 0))
        exit(EXIT_SUCCESS);

    ///////////////////////////////////////
    // Display plugin file and arguments //
    ///////////////////////////////////////

    if (application.verbose)
    {
        if (!application.plugin.built_in)
            PRINT_("Plugin file: " COLOR_STRING "%s" COLOR_RESET "\n", application.args.inputs[0]);
        else
            PRINT("Plugin is built-in\n");

        if (application.plugin.argc > 0)
        {
            PRINT_("  " COLOR_NUMBER "%i" COLOR_RESET " arguments given:\n",
                    application.plugin.argc);
            for (int i = 0; i < application.plugin.argc; i++)
                PRINT_("    [" COLOR_NUMBER "%i" COLOR_RESET "]: "
                        COLOR_STRING "%s" COLOR_RESET "\n", i, application.plugin.argv[i]);
        }
        else
            PRINT("  no arguments given\n");

        PRINT("\n");
    }

    //////////////////////
    // Load plugin file //
    //////////////////////

#ifdef STATION_IS_DLFCN_SUPPORTED
    if (!application.plugin.built_in)
    {
        application.plugin.handle = dlopen(application.args.inputs[0], RTLD_NOW | RTLD_LOCAL);
        if (application.plugin.handle == NULL)
        {
            ERROR_("couldn't load plugin " COLOR_STRING "%s" COLOR_RESET
                    " (%s)", application.args.inputs[0], dlerror());
            exit(STATION_APP_ERROR_PLUGIN);
        }
        AT_EXIT(exit_unload_plugin);
    }
#else
    if (!application.plugin.built_in)
    {
        ERROR("couldn't load plugin because shared libraries are not supported");
        exit(STATION_APP_ERROR_PLUGIN);
    }
#endif

    //////////////////////////
    // Obtain plugin vtable //
    //////////////////////////

#ifdef STATION_IS_DLFCN_SUPPORTED
    if (!application.plugin.built_in)
    {
        application.plugin.vtable = dlsym(application.plugin.handle,
                STRINGIFY(STATION_PLUGIN_VTABLE_OBJECT));
        if (application.plugin.vtable == NULL)
        {
            ERROR("couldn't obtain plugin vtable");
            exit(STATION_APP_ERROR_PLUGIN);
        }

        check_plugin();
    }
#endif

    if (application.verbose)
    {
        PRINT_("Plugin name: " COLOR_STRING "%s" COLOR_RESET "\n",
                application.plugin.vtable->info.name);

        if (application.plugin.vtable->info.description != NULL)
            PRINT_("Plugin description: " COLOR_STRING "%s" COLOR_RESET "\n",
                    application.plugin.vtable->info.description);

        PRINT("\n");
    }

    ///////////////////////////////
    // Display plugin usage help //
    ///////////////////////////////

    if (application.args.help_given)
    {
        if (application.verbose)
        {
            PRINT(COLOR_OUTPUT_SEGMENT OUTPUT_SEGMENT_BEGIN_HELP "\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n" COLOR_RESET);
            fflush(stderr);

            AT_EXIT(exit_end_plugin_help_fn_output);
            AT_QUICK_EXIT(exit_end_plugin_help_fn_output);
        }

        application.plugin.vtable->func.help(application.plugin.argc, application.plugin.argv);

        if (application.verbose)
            exit_end_plugin_help_fn_output();

        exit(EXIT_SUCCESS);
    }

    ////////////////////////////////////////
    // Call plugin configuration function //
    ////////////////////////////////////////

    application.plugin.configuration.std_signals_used = &application.signal.std_set;
    application.plugin.configuration.rt_signals_used = &application.signal.rt_set;

    if (application.verbose)
    {
        PRINT(COLOR_OUTPUT_SEGMENT OUTPUT_SEGMENT_BEGIN_CONF "\n");
        PRINT(OUTPUT_SEGMENT_SEPARATOR "\n" COLOR_RESET);
        fflush(stderr);

        AT_EXIT(exit_end_plugin_conf_fn_output);
        AT_QUICK_EXIT(exit_end_plugin_conf_fn_output);
    }

    application.plugin.vtable->func.conf(&application.plugin.configuration,
            application.plugin.argc, application.plugin.argv);

    if (application.verbose)
        exit_end_plugin_conf_fn_output();

    ///////////////////////////////////
    // Process application arguments //
    ///////////////////////////////////

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
#  define CATCH_SIGNAL(signame) do {                        \
    if (application.args.signame##_given) {                 \
        application.signal.std_set.signal_##signame = true; \
        application.signal.management_used = true;          \
    } else if (application.signal.std_set.signal_##signame) \
        application.signal.management_used = true; } while (0)

    CATCH_SIGNAL(SIGINT);
    CATCH_SIGNAL(SIGQUIT);
    CATCH_SIGNAL(SIGTERM);

    CATCH_SIGNAL(SIGCHLD);
    CATCH_SIGNAL(SIGCONT);
    CATCH_SIGNAL(SIGTSTP);
    CATCH_SIGNAL(SIGXCPU);
    CATCH_SIGNAL(SIGXFSZ);

    CATCH_SIGNAL(SIGPIPE);
    CATCH_SIGNAL(SIGPOLL);
    CATCH_SIGNAL(SIGURG);

    CATCH_SIGNAL(SIGALRM);
    CATCH_SIGNAL(SIGVTALRM);
    CATCH_SIGNAL(SIGPROF);

    CATCH_SIGNAL(SIGHUP);
    CATCH_SIGNAL(SIGTTIN);
    CATCH_SIGNAL(SIGTTOU);
    CATCH_SIGNAL(SIGWINCH);

    CATCH_SIGNAL(SIGUSR1);
    CATCH_SIGNAL(SIGUSR2);

#  undef CATCH_SIGNAL

    for (unsigned i = 0; i < application.args.SIGRTMIN_given; i++)
        application.signal.rt_set.signal_SIGRTMIN[application.args.SIGRTMIN_arg[i]] = true;

    for (unsigned i = 0; i < application.args.SIGRTMAX_given; i++)
        application.signal.rt_set.signal_SIGRTMIN[application.args.SIGRTMAX_arg[i] + SIGRTMAX-SIGRTMIN] = true;

    if (application.args.SIGRTMIN_given || application.args.SIGRTMAX_given)
        application.signal.management_used = true;

    for (int i = 0; i <= SIGRTMAX-SIGRTMIN; i++)
    {
        if (application.signal.rt_set.signal_SIGRTMIN[i])
            application.signal.management_used = true;
    }
#endif

    application.concurrent_processing.contexts.num_contexts =
        application.plugin.configuration.num_concurrent_processing_contexts_used;
    if (application.concurrent_processing.contexts.num_contexts > application.args.threads_given)
        application.concurrent_processing.contexts.num_contexts = application.args.threads_given;

#ifdef STATION_IS_OPENCL_SUPPORTED
    application.opencl.contexts.num_contexts =
        application.plugin.configuration.num_opencl_contexts_used;
    if (application.opencl.contexts.num_contexts > application.args.cl_context_given)
        application.opencl.contexts.num_contexts = application.args.cl_context_given;
#endif

#ifdef STATION_IS_SDL_SUPPORTED
    if (application.args.no_sdl_given)
        application.plugin.configuration.sdl_is_used = false;
#else
    application.plugin.configuration.sdl_is_used = false;
#endif

    application.file.count = application.plugin.configuration.num_files_used;
    if (application.file.count > application.args.file_given)
        application.file.count = application.args.file_given;

    application.library.count = application.plugin.configuration.num_libraries_used;
    if (application.library.count > application.args.library_given)
        application.library.count = application.args.library_given;

    ///////////////////////////////////////
    // Display application configuration //
    ///////////////////////////////////////

    if (application.verbose)
    {
        PRINT("\n");

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
        if (application.signal.management_used)
        {
            PRINT("Standard signals:");

#  define PRINT_SIGNAL(signame) do {                            \
            if (application.signal.std_set.signal_##signame)    \
                PRINT_(" " COLOR_SIGNAL "%s" COLOR_RESET, &#signame[3]); } while (0)

            PRINT_SIGNAL(SIGINT);
            PRINT_SIGNAL(SIGQUIT);
            PRINT_SIGNAL(SIGTERM);

            PRINT_SIGNAL(SIGCHLD);
            PRINT_SIGNAL(SIGCONT);
            PRINT_SIGNAL(SIGTSTP);
            PRINT_SIGNAL(SIGXCPU);
            PRINT_SIGNAL(SIGXFSZ);

            PRINT_SIGNAL(SIGPIPE);
            PRINT_SIGNAL(SIGPOLL);
            PRINT_SIGNAL(SIGURG);

            PRINT_SIGNAL(SIGALRM);
            PRINT_SIGNAL(SIGVTALRM);
            PRINT_SIGNAL(SIGPROF);

            PRINT_SIGNAL(SIGHUP);
            PRINT_SIGNAL(SIGTTIN);
            PRINT_SIGNAL(SIGTTOU);
            PRINT_SIGNAL(SIGWINCH);

            PRINT_SIGNAL(SIGUSR1);
            PRINT_SIGNAL(SIGUSR2);

#  undef PRINT_SIGNAL

            PRINT("\n");

            PRINT("Real-time signals:");

            int printed = 0;
            for (int i = 0; i <= SIGRTMAX-SIGRTMIN; i++)
                if (application.signal.rt_set.signal_SIGRTMIN[i])
                {
                    if (printed++ % 11 == 0)
                        PRINT("\n  ");

                    PRINT(" " COLOR_SIGNAL);

                    if (i <= (SIGRTMAX-SIGRTMIN)/2)
                        PRINT_("MIN+%i", i);
                    else
                        PRINT_("MAX-%i", (SIGRTMAX-SIGRTMIN) - i);

                    PRINT(COLOR_RESET);
                }

            PRINT("\n");
        }
#endif

        if ((application.concurrent_processing.contexts.num_contexts > 0) || (application.args.threads_given > 0))
        {
            PRINT_("Concurrent processing contexts: " COLOR_NUMBER "%lu" COLOR_RESET,
                    (unsigned long)application.concurrent_processing.contexts.num_contexts);

            if (application.args.threads_given > application.concurrent_processing.contexts.num_contexts)
                PRINT_(" (extra " COLOR_NUMBER "%lu" COLOR_RESET " ignored)",
                        (unsigned long)(application.args.threads_given -
                            application.concurrent_processing.contexts.num_contexts));

            PRINT("\n");

            for (size_t i = 0; i < application.concurrent_processing.contexts.num_contexts; i++)
            {
                PRINT_("  [" COLOR_NUMBER "%lu" COLOR_RESET "]: ", (unsigned long)i);

                int threads = application.args.threads_arg[i];

                if (threads > 0)
                    PRINT_(COLOR_NUMBER "%i" COLOR_RESET " thread%s (waiting on condition variable)\n",
                            threads, threads > 1 ? "s" : "");
                else if (threads < 0)
                    PRINT_(COLOR_NUMBER "%i" COLOR_RESET " thread%s (busy waiting)\n",
                            -threads, -threads > 1 ? "s" : "");
                else
                    PRINT("no threads\n");
            }
        }

#ifdef STATION_IS_OPENCL_SUPPORTED
        if ((application.opencl.contexts.num_contexts > 0) || (application.args.cl_context_given > 0))
        {
            PRINT_("OpenCL contexts: " COLOR_NUMBER "%lu" COLOR_RESET,
                    (unsigned long)application.opencl.contexts.num_contexts);

            if (application.args.cl_context_given > application.opencl.contexts.num_contexts)
                PRINT_(" (extra " COLOR_NUMBER "%lu" COLOR_RESET " ignored)",
                        (unsigned long)(application.args.cl_context_given - application.opencl.contexts.num_contexts));

            PRINT("\n");

            for (size_t i = 0; i < application.opencl.contexts.num_contexts; i++)
            {
                cl_uint platform_idx = strtoul(
                        application.args.cl_context_arg[i], (char**)NULL, 16);

                PRINT_("  [" COLOR_NUMBER "%lu" COLOR_RESET "]: "
                        "platform #" COLOR_NUMBER "%x" COLOR_RESET ", ", (unsigned long)i, platform_idx);

                const char *device_mask = strchr(application.args.cl_context_arg[i], ':');
                if (device_mask != NULL)
                    device_mask++;

                if (device_mask == NULL)
                    PRINT("all devices");
                else
                    PRINT_("device mask " COLOR_NUMBER "%s" COLOR_RESET, device_mask);

                PRINT("\n");
            }
        }
#endif

#ifdef STATION_IS_SDL_SUPPORTED
        if (application.plugin.configuration.sdl_is_used)
        {
            PRINT("SDL subsystems:");

            if ((application.plugin.configuration.sdl_init_flags &
                        SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING)
                PRINT(" " COLOR_SDL_SUBSYSTEM "EVERYTHING" COLOR_RESET "\n");
            else
            {
#  define PRINT_SUBSYSTEM(name) \
                if ((application.plugin.configuration.sdl_init_flags &  \
                            SDL_INIT_##name) == SDL_INIT_##name)        \
                    PRINT(" " COLOR_SDL_SUBSYSTEM #name COLOR_RESET);

                PRINT_SUBSYSTEM(TIMER)
                PRINT_SUBSYSTEM(AUDIO)
                PRINT_SUBSYSTEM(VIDEO)
                PRINT_SUBSYSTEM(JOYSTICK)
                PRINT_SUBSYSTEM(HAPTIC)
                PRINT_SUBSYSTEM(GAMECONTROLLER)
                PRINT_SUBSYSTEM(EVENTS)

#  undef PRINT_SUBSYSTEM

                PRINT("\n");
            }
        }
#endif

        if ((application.file.count > 0) || (application.args.file_given > 0))
        {
            PRINT_("Files: " COLOR_NUMBER "%lu" COLOR_RESET, (unsigned long)application.file.count);

            if (application.args.file_given > application.file.count)
                PRINT_(" (extra " COLOR_NUMBER "%lu" COLOR_RESET " ignored)",
                        (unsigned long)(application.args.file_given - application.file.count));

            PRINT("\n");

            for (size_t i = 0; i < application.file.count; i++)
                PRINT_("  [" COLOR_NUMBER "%lu" COLOR_RESET "]: "
                        COLOR_STRING "%s" COLOR_RESET "\n", (unsigned long)i, application.args.file_arg[i]);
        }

        PRINT("\n");

#ifdef STATION_IS_DLFCN_SUPPORTED
        if ((application.library.count > 0) || (application.args.library_given > 0))
        {
            PRINT_("Libraries: " COLOR_NUMBER "%lu" COLOR_RESET, (unsigned long)application.library.count);

            if (application.args.library_given > application.library.count)
                PRINT_(" (extra " COLOR_NUMBER "%lu" COLOR_RESET " ignored)",
                        (unsigned long)(application.args.library_given - application.library.count));

            PRINT("\n");

            for (size_t i = 0; i < application.library.count; i++)
                PRINT_("  [" COLOR_NUMBER "%lu" COLOR_RESET "]: "
                        COLOR_STRING "%s" COLOR_RESET "\n", (unsigned long)i, application.args.library_arg[i]);
        }
#endif
    }

    ///////////////////////////////
    // Create buffers from files //
    ///////////////////////////////

    if (application.file.count > 0)
    {
        application.file.streams = malloc(
                sizeof(*application.file.streams) * application.file.count);
        if (application.file.streams == NULL)
        {
            ERROR("couldn't allocate array of file streams");
            exit(STATION_APP_ERROR_MALLOC);
        }

        for (size_t i = 0; i < application.file.count; i++)
            application.file.streams[i] = NULL;

        AT_EXIT(exit_close_files);

        for (size_t i = 0; i < application.file.count; i++)
        {
            application.file.streams[i] = fopen(application.args.file_arg[i], "rb");

            if (application.file.streams[i] == NULL)
            {
                ERROR_("couldn't open file ["
                        COLOR_NUMBER "%lu" COLOR_RESET "]: "
                        COLOR_STRING "%s" COLOR_RESET "\n",
                        (unsigned long)i, application.args.file_arg[i]);
                exit(STATION_APP_ERROR_FILE);
            }
        }
    }

    ////////////////////////////
    // Load shared libraries ///
    ////////////////////////////

#ifdef STATION_IS_DLFCN_SUPPORTED
    if (application.library.count > 0)
    {
        application.library.handles = malloc(
                sizeof(*application.library.handles) * application.library.count);
        if (application.library.handles == NULL)
        {
            ERROR("couldn't allocate array of shared library handles");
            exit(STATION_APP_ERROR_MALLOC);
        }

        for (size_t i = 0; i < application.library.count; i++)
            application.library.handles[i] = NULL;

        AT_EXIT(exit_close_libraries);

        for (size_t i = 0; i < application.library.count; i++)
        {
            application.library.handles[i] = dlopen(application.args.library_arg[i], RTLD_LAZY | RTLD_GLOBAL);

            if (application.library.handles[i] == NULL)
            {
                ERROR_("couldn't open shared library ["
                        COLOR_NUMBER "%lu" COLOR_RESET "]: "
                        COLOR_STRING "%s" COLOR_RESET "\n",
                        (unsigned long)i, application.args.library_arg[i]);
                exit(STATION_APP_ERROR_LIBRARY);
            }
        }
    }
#endif

    ////////////////////////////////////
    // Start signal management thread //
    ////////////////////////////////////

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    if (application.signal.management_used)
    {
        application.signal.management_context =
            station_signal_management_thread_start(
                    &application.signal.std_set, &application.signal.rt_set,
                    application.plugin.configuration.signal_handler,
                    application.plugin.configuration.signal_handler_data);

        if (application.signal.management_context == NULL)
        {
            ERROR("couldn't configure signal management");
            exit(STATION_APP_ERROR_SIGNAL);
        }

        AT_EXIT(exit_stop_signal_management_thread);
        AT_QUICK_EXIT(exit_stop_signal_management_thread);
    }
#endif

    //////////////////////////////////////////
    // Create concurrent processing threads //
    //////////////////////////////////////////

    if (application.concurrent_processing.contexts.num_contexts > 0)
    {
        application.concurrent_processing.contexts.contexts = malloc(
                sizeof(*application.concurrent_processing.contexts.contexts) *
                application.concurrent_processing.contexts.num_contexts);
        if (application.concurrent_processing.contexts.contexts == NULL)
        {
            ERROR("couldn't allocate array of concurrent processing contexts");
            exit(STATION_APP_ERROR_MALLOC);
        }

        for (size_t i = 0; i < application.concurrent_processing.contexts.num_contexts; i++)
            application.concurrent_processing.contexts.contexts[i].state = NULL;

        AT_EXIT(exit_destroy_concurrent_processing_contexts);
        AT_QUICK_EXIT(exit_destroy_concurrent_processing_contexts);

        for (size_t i = 0; i < application.concurrent_processing.contexts.num_contexts; i++)
        {
            station_threads_number_t num_threads;
            bool busy_wait;

            if (application.args.threads_arg[i] >= 0)
            {
                num_threads = application.args.threads_arg[i];
                busy_wait = false;
            }
            else
            {
                num_threads = -application.args.threads_arg[i];
                busy_wait = true;
            }

            int code = station_concurrent_processing_initialize_context(
                    &application.concurrent_processing.contexts.contexts[i],
                    num_threads, busy_wait);

            if (code != 0)
            {
                ERROR_("couldn't create concurrent processing context ["
                        COLOR_NUMBER "%lu" COLOR_RESET "], got error "
                        COLOR_ERROR "%i" COLOR_RESET, (unsigned long)i, code);
                exit(STATION_APP_ERROR_THREADS);
            }
        }
    }

    ////////////////////////////
    // Create OpenCL contexts //
    ////////////////////////////

#ifdef STATION_IS_OPENCL_SUPPORTED
    if (application.opencl.contexts.num_contexts > 0)
    {
        AT_EXIT(exit_release_opencl_contexts);
        AT_QUICK_EXIT(exit_release_opencl_contexts_quick);

        prepare_opencl_tmp_arrays();
        create_opencl_contexts();
        free_opencl_tmp_arrays();
    }
#endif

    ///////////////////////////////
    // Initialize SDL subsystems //
    ///////////////////////////////

#ifdef STATION_IS_SDL_SUPPORTED
    if (application.plugin.configuration.sdl_is_used)
    {
        if (SDL_Init(application.plugin.configuration.sdl_init_flags) < 0)
        {
            ERROR("couldn't initialize SDL subsystems");
            exit(STATION_APP_ERROR_SDL);
        }

        AT_EXIT(exit_quit_sdl);
        AT_QUICK_EXIT(exit_quit_sdl);
    }
#endif
}

static void check_plugin(void)
{
    if (application.plugin.vtable->format.magic != STATION_PLUGIN_MAGIC)
    {
        ERROR_("plugin magic number (" COLOR_VERSION "0x%X" COLOR_RESET
                ") is wrong (must be " COLOR_VERSION "0x%X" COLOR_RESET ")",
                application.plugin.vtable->format.magic, STATION_PLUGIN_MAGIC);
        exit(STATION_APP_ERROR_PLUGIN);
    }

    if (application.plugin.vtable->format.version != STATION_PLUGIN_VERSION)
    {
        ERROR_("plugin version (" COLOR_VERSION "%u" COLOR_RESET
                ") is different from application version (" COLOR_VERSION "%u" COLOR_RESET ")",
                application.plugin.vtable->format.version, STATION_PLUGIN_VERSION);
        exit(STATION_APP_ERROR_PLUGIN);
    }

    if ((application.plugin.vtable->func.help == NULL) ||
            (application.plugin.vtable->func.conf == NULL) ||
            (application.plugin.vtable->func.init == NULL) ||
            (application.plugin.vtable->func.final == NULL))
    {
        ERROR("plugin vtable contains NULL function pointers");
        exit(STATION_APP_ERROR_PLUGIN);
    }
    else if (application.plugin.vtable->info.name == NULL)
    {
        ERROR("plugin name string is NULL");
        exit(STATION_APP_ERROR_PLUGIN);
    }
}

static void exit_release_args(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    args_parser_free(&application.args);
}

#ifdef STATION_IS_DLFCN_SUPPORTED
static void exit_unload_plugin(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    dlclose(application.plugin.handle);
}
#endif

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
static void exit_stop_signal_management_thread(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    station_signal_management_thread_stop(application.signal.management_context);
}
#endif

static void exit_destroy_concurrent_processing_contexts(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    if (application.concurrent_processing.contexts.contexts != NULL)
        for (size_t i = 0; i < application.concurrent_processing.contexts.num_contexts; i++)
            station_concurrent_processing_destroy_context(&application.concurrent_processing.contexts.contexts[i]);

    free(application.concurrent_processing.contexts.contexts);
}

#ifdef STATION_IS_SDL_SUPPORTED
static void exit_quit_sdl(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    SDL_Quit();
}
#endif

#ifdef STATION_IS_DLFCN_SUPPORTED
static void exit_close_libraries(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    if (application.library.handles != NULL)
        for (size_t i = 0; i < application.library.count; i++)
            if (application.library.handles[i] != NULL)
                dlclose(application.library.handles[i]);

    free(application.library.handles);
}
#endif

static void exit_close_files(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    if (application.file.streams != NULL)
        for (size_t i = 0; i < application.file.count; i++)
            if (application.file.streams[i] != NULL)
                fclose(application.file.streams[i]);

    free(application.file.streams);
}

static void exit_end_plugin_help_fn_output(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    static bool job_done = false;
    if (job_done)
        return;
    job_done = true;

    fflush(stdout);
    fflush(stderr);

    PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
    PRINT(OUTPUT_SEGMENT_END_HELP "\n" COLOR_RESET);
}

static void exit_end_plugin_conf_fn_output(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    static bool job_done = false;
    if (job_done)
        return;
    job_done = true;

    fflush(stdout);
    fflush(stderr);

    PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
    PRINT(OUTPUT_SEGMENT_END_CONF "\n" COLOR_RESET);
}

/*****************************************************************************/

static int run(void)
{
    ///////////////////////
    // Initialize plugin //
    ///////////////////////

    {
        station_plugin_init_func_inputs_t plugin_init_func_inputs = {
            .cmdline = application.plugin.configuration.cmdline,
            .std_signals = &application.signal.std_set,
            .rt_signals = &application.signal.rt_set,
            .signal_handler_data = application.plugin.configuration.signal_handler_data,
            .num_files = application.file.count,
            .files = application.file.streams,
            .num_libraries = application.library.count,
            .libraries = application.library.handles,
            .concurrent_processing_contexts = &application.concurrent_processing.contexts,
            .opencl_contexts = &application.opencl.contexts,
            .sdl_is_available = application.plugin.configuration.sdl_is_used,
        };

        station_plugin_init_func_outputs_t plugin_init_func_outputs = {0};

        if (application.verbose)
        {
            PRINT(COLOR_OUTPUT_SEGMENT OUTPUT_SEGMENT_BEGIN_INIT "\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n" COLOR_RESET);
            fflush(stderr);

            AT_EXIT(exit_end_plugin_init_fn_output);
            AT_QUICK_EXIT(exit_end_plugin_init_fn_output);
        }

        application.plugin.vtable->func.init(&plugin_init_func_inputs, &plugin_init_func_outputs);

        if (application.verbose)
            exit_end_plugin_init_fn_output();

        application.plugin.resources = plugin_init_func_outputs.plugin_resources;
        application.fsm.state = plugin_init_func_outputs.fsm_initial_state;
        application.fsm.data = plugin_init_func_outputs.fsm_data;

        AT_EXIT(exit_finalize_plugin);
        AT_QUICK_EXIT(exit_finalize_plugin_quick);
    }

    //////////////////////////////////////
    // Execute the finite state machine //
    //////////////////////////////////////

    if (application.fsm.state.sfunc != NULL)
    {
        if (application.verbose)
        {
            PRINT(COLOR_OUTPUT_SEGMENT OUTPUT_SEGMENT_BEGIN_EXEC "\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n" COLOR_RESET);

            AT_EXIT(exit_end_plugin_exec_fn_output);
            AT_QUICK_EXIT(exit_end_plugin_exec_fn_output);
        }

        while (application.fsm.state.sfunc != NULL)
            application.fsm.state.sfunc(&application.fsm.state, application.fsm.data);

        if (application.verbose)
            exit_end_plugin_exec_fn_output();
    }

    return finalize(false);
}

static int finalize(bool quick)
{
    /////////////////////
    // Finalize plugin //
    /////////////////////

    static bool job_done = false;
    if (job_done)
        return 0; // ignored
    job_done = true;

    if (application.verbose)
    {
        PRINT(COLOR_OUTPUT_SEGMENT OUTPUT_SEGMENT_BEGIN_FINAL "\n");
        PRINT(OUTPUT_SEGMENT_SEPARATOR "\n" COLOR_RESET);
        fflush(stderr);
    }

    int exit_code = application.plugin.vtable->func.final(application.plugin.resources, quick);

    fflush(stdout);
    fflush(stderr);

    if (application.verbose)
    {
        PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
        PRINT(OUTPUT_SEGMENT_END_FINAL "\n" COLOR_RESET);
    }

    return exit_code;
}

static void exit_finalize_plugin(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    finalize(false);
}

static void exit_finalize_plugin_quick(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    finalize(true);
}

static void exit_end_plugin_init_fn_output(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    static bool job_done = false;
    if (job_done)
        return;
    job_done = true;

    fflush(stdout);
    fflush(stderr);

    PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
    PRINT(OUTPUT_SEGMENT_END_INIT "\n" COLOR_RESET);
}

static void exit_end_plugin_exec_fn_output(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    static bool job_done = false;
    if (job_done)
        return;
    job_done = true;

    fflush(stdout);
    fflush(stderr);

    PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
    PRINT(OUTPUT_SEGMENT_END_EXEC "\n" COLOR_RESET);
}

/*****************************************************************************/

#ifdef STATION_IS_OPENCL_SUPPORTED

static void display_opencl_listing(void)
{
    cl_int ret;

    cl_uint num_platforms;

    ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &num_platforms);
    if (ret != CL_SUCCESS)
    {
        ERROR("couldn't obtain number of OpenCL platforms");
        exit(STATION_APP_ERROR_OPENCL);
    }

    PRINT_("Number of OpenCL platforms: " COLOR_NUMBER "%u" COLOR_RESET "\n", num_platforms);

    cl_platform_id *platform_list = malloc(sizeof(cl_platform_id) * num_platforms);
    if (platform_list == NULL)
    {
        ERROR("couldn't allocate OpenCL platform list");
        exit(STATION_APP_ERROR_MALLOC);
    }

    ret = clGetPlatformIDs(num_platforms, platform_list, (cl_uint*)NULL);
    if (ret != CL_SUCCESS)
    {
        ERROR("couldn't obtain OpenCL platform list");
        free(platform_list);
        exit(STATION_APP_ERROR_OPENCL);
    }

    cl_uint device_list_size = 0;
    cl_device_id *device_list = NULL;

    for (cl_uint platform_idx = 0; platform_idx < num_platforms; platform_idx++)
    {
        {
            char name[256] = {0};

            ret = clGetPlatformInfo(platform_list[platform_idx],
                    CL_PLATFORM_NAME, sizeof(name)-1, name, (size_t*)NULL);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain name of OpenCL platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                free(platform_list);
                free(device_list);
                exit(STATION_APP_ERROR_OPENCL);
            }

            PRINT_("#" COLOR_NUMBER "%x" COLOR_RESET ": "
                    COLOR_STRING "%s" COLOR_RESET "\n", platform_idx, name);
        }

        if (application.args.cl_list_arg == cl_list_arg_devices)
        {
            cl_uint num_devices;

            ret = clGetDeviceIDs(platform_list[platform_idx],
                    CL_DEVICE_TYPE_DEFAULT, 0, (cl_device_id*)NULL, &num_devices);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain number of OpenCL devices for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                free(platform_list);
                free(device_list);
                exit(STATION_APP_ERROR_OPENCL);
            }

            PRINT_("  number of devices: " COLOR_NUMBER "%u" COLOR_RESET "\n", num_devices);

            if (num_devices > device_list_size)
            {
                cl_device_id *new_device_list = realloc(device_list,
                        sizeof(cl_device_id) * num_devices);
                if (new_device_list == NULL)
                {
                    ERROR("couldn't allocate OpenCL device list");
                    free(platform_list);
                    free(device_list);
                    exit(STATION_APP_ERROR_MALLOC);
                }

                device_list_size = num_devices;
                device_list = new_device_list;
            }

            ret = clGetDeviceIDs(platform_list[platform_idx],
                    CL_DEVICE_TYPE_DEFAULT, num_devices, device_list, (cl_uint*)NULL);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain OpenCL device list for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                free(platform_list);
                free(device_list);
                exit(STATION_APP_ERROR_OPENCL);
            }

            for (cl_uint device_idx = 0; device_idx < num_devices; device_idx++)
            {
                char name[256] = {0};

                clGetDeviceInfo(device_list[device_idx],
                        CL_DEVICE_NAME, sizeof(name)-1, name, (size_t*)NULL);
                if (ret != CL_SUCCESS)
                {
                    ERROR_("couldn't obtain name of OpenCL device #"
                            COLOR_NUMBER "%x" COLOR_RESET " for platform #"
                            COLOR_NUMBER "%x" COLOR_RESET, device_idx, platform_idx);
                    free(platform_list);
                    free(device_list);
                    exit(STATION_APP_ERROR_OPENCL);
                }

                PRINT_("    #" COLOR_NUMBER "%x" COLOR_RESET ": "
                        COLOR_STRING "%s" COLOR_RESET "\n", device_idx, name);
            }
        }
    }

    free(platform_list);
    free(device_list);
}

static void prepare_opencl_tmp_arrays(void)
{
    cl_int ret;

    // Obtain platform list
    ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &application.opencl.tmp.num_platforms);
    if (ret != CL_SUCCESS)
    {
        ERROR("couldn't obtain number of OpenCL platforms");
        exit(STATION_APP_ERROR_OPENCL);
    }

    application.opencl.tmp.platform_list = malloc(
            sizeof(cl_platform_id) * application.opencl.tmp.num_platforms);
    if (application.opencl.tmp.platform_list == NULL)
    {
        ERROR("couldn't allocate OpenCL platform list");
        exit(STATION_APP_ERROR_MALLOC);
    }

    ret = clGetPlatformIDs(application.opencl.tmp.num_platforms,
            application.opencl.tmp.platform_list, (cl_uint*)NULL);
    if (ret != CL_SUCCESS)
    {
        ERROR("couldn't obtain OpenCL platform list");
        exit(STATION_APP_ERROR_OPENCL);
    }

    // Obtain sizes of device lists
    application.opencl.tmp.num_devices = malloc(
            sizeof(cl_uint) * application.opencl.tmp.num_platforms);
    if (application.opencl.tmp.num_devices == NULL)
    {
        ERROR("couldn't allocate list of numbers of OpenCL devices");
        exit(STATION_APP_ERROR_MALLOC);
    }

    for (cl_uint platform_idx = 0; platform_idx < application.opencl.tmp.num_platforms; platform_idx++)
    {
        ret = clGetDeviceIDs(application.opencl.tmp.platform_list[platform_idx], CL_DEVICE_TYPE_DEFAULT,
                0, (cl_device_id*)NULL, &application.opencl.tmp.num_devices[platform_idx]);
        if (ret != CL_SUCCESS)
        {
            ERROR_("couldn't obtain number of OpenCL devices for platform #"
                    COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
            exit(STATION_APP_ERROR_OPENCL);
        }
    }

    // Obtain device lists
    application.opencl.tmp.device_list = malloc(
            sizeof(cl_device_id*) * application.opencl.tmp.num_platforms);
    if (application.opencl.tmp.device_list == NULL)
    {
        ERROR("couldn't allocate list of OpenCL device lists");
        exit(STATION_APP_ERROR_MALLOC);
    }

    for (cl_uint platform_idx = 0; platform_idx < application.opencl.tmp.num_platforms; platform_idx++)
        application.opencl.tmp.device_list[platform_idx] = NULL;

    for (cl_uint platform_idx = 0; platform_idx < application.opencl.tmp.num_platforms; platform_idx++)
    {
        application.opencl.tmp.device_list[platform_idx] = malloc(
                sizeof(cl_device_id) * application.opencl.tmp.num_devices[platform_idx]);
        if (application.opencl.tmp.device_list[platform_idx] == NULL)
        {
            ERROR_("couldn't allocate OpenCL device list for platform #"
                    COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
            exit(STATION_APP_ERROR_MALLOC);
        }

        ret = clGetDeviceIDs(application.opencl.tmp.platform_list[platform_idx], CL_DEVICE_TYPE_DEFAULT,
                application.opencl.tmp.num_devices[platform_idx],
                application.opencl.tmp.device_list[platform_idx], (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            ERROR_("couldn't obtain OpenCL device list for platform #"
                    COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
            exit(STATION_APP_ERROR_OPENCL);
        }
    }
}

static void create_opencl_contexts(void)
{
    // Allocate array of contexts
    application.opencl.contexts.contexts = malloc(sizeof(cl_context) *
            application.opencl.contexts.num_contexts);
    if (application.opencl.contexts.contexts == NULL)
    {
        ERROR("couldn't allocate array of OpenCL platform contexts");
        exit(STATION_APP_ERROR_MALLOC);
    }

    for (cl_uint i = 0; i < application.opencl.contexts.num_contexts; i++)
        application.opencl.contexts.contexts[i] = NULL;

    // Allocate array of platforms
    application.opencl.contexts.context_info = malloc(
            sizeof(*application.opencl.contexts.context_info) *
            application.opencl.contexts.num_contexts);
    if (application.opencl.contexts.context_info == NULL)
    {
        ERROR("couldn't allocate array of OpenCL platforms");
        exit(STATION_APP_ERROR_MALLOC);
    }

    for (cl_uint i = 0; i < application.opencl.contexts.num_contexts; i++)
    {
        application.opencl.contexts.context_info[i].platform_id = NULL;
        application.opencl.contexts.context_info[i].device_ids = NULL;
        application.opencl.contexts.context_info[i].num_devices = 0;
    }

    // Create contexts
    for (cl_uint i = 0; i < application.opencl.contexts.num_contexts; i++)
    {
        // Extract platform index
        cl_uint platform_idx = strtoul(application.args.cl_context_arg[i], (char**)NULL, 16);
        if (platform_idx >= application.opencl.tmp.num_platforms)
        {
            ERROR_("OpenCL platform index #" COLOR_NUMBER "%x" COLOR_RESET
                    " is greater or equal to number of available platforms ("
                    COLOR_NUMBER "%u" COLOR_RESET ")",
                    platform_idx, application.opencl.tmp.num_platforms);
            exit(STATION_APP_ERROR_ARGUMENTS);
        }

        // Extract device mask
        const char *device_mask = strchr(application.args.cl_context_arg[i], ':');
        size_t device_mask_len = 0;
        if (device_mask != NULL)
        {
            device_mask++;
            device_mask_len = strlen(device_mask);
        }

        // Copy platform ID
        application.opencl.contexts.context_info[i].platform_id =
            application.opencl.tmp.platform_list[platform_idx];

        // Compute number of devices from device mask
        if (device_mask == NULL)
            application.opencl.contexts.context_info[i].num_devices =
                application.opencl.tmp.num_devices[platform_idx];
        else
        {
            for (size_t m = 0; m < device_mask_len; m++)
            {
                int chr = device_mask[m];

                if (('A' <= chr) && (chr <= 'F'))
                    chr -= 'A' - 10;
                else if (('a' <= chr) && (chr <= 'f'))
                    chr -= 'a' - 10;
                else
                    chr -= '0';

                application.opencl.contexts.context_info[i].num_devices +=
                    (int[16]){0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4}[chr];
            }

            if (application.opencl.contexts.context_info[i].num_devices >
                    application.opencl.tmp.num_devices[platform_idx])
            {
                ERROR_("OpenCL device mask " COLOR_NUMBER "%s" COLOR_RESET
                        " enables more devices than available ("
                        COLOR_NUMBER "%u" COLOR_RESET ") on platform #"
                        COLOR_NUMBER "%x" COLOR_RESET,
                        device_mask, application.opencl.tmp.num_devices[platform_idx],
                        platform_idx);
                exit(STATION_APP_ERROR_ARGUMENTS);
            }
        }

        // Allocate array of devices
        application.opencl.contexts.context_info[i].device_ids = malloc(
                sizeof(cl_device_id) * application.opencl.contexts.context_info[i].num_devices);
        if (application.opencl.contexts.context_info[i].device_ids == NULL)
        {
            ERROR("couldn't allocate array of OpenCL devices");
            exit(STATION_APP_ERROR_MALLOC);
        }

        // Copy device IDs
        if (device_mask == NULL)
        {
            for (cl_uint device_idx = 0; device_idx <
                    application.opencl.tmp.num_devices[platform_idx]; device_idx++)
                application.opencl.contexts.context_info[i].device_ids[device_idx] =
                    application.opencl.tmp.device_list[platform_idx][device_idx];
        }
        else
        {
            cl_uint j = 0;
            for (size_t m = 0; m < device_mask_len; m++)
            {
                int chr = device_mask[(device_mask_len - 1) - m];

                if (('A' <= chr) && (chr <= 'F'))
                    chr -= 'A' - 10;
                else if (('a' <= chr) && (chr <= 'f'))
                    chr -= 'a' - 10;
                else
                    chr -= '0';

                for (cl_uint device_idx = 4 * m; device_idx < 4 * (m + 1); device_idx++)
                {
                    if (chr & 1)
                    {
                        if (device_idx >= application.opencl.tmp.num_devices[platform_idx])
                        {
                            ERROR_("OpenCL device index #" COLOR_NUMBER "%x" COLOR_RESET
                                    " is greater or equal to number of available devices ("
                                    COLOR_NUMBER "%u" COLOR_RESET ") on platform #"
                                    COLOR_NUMBER "%x" COLOR_RESET,
                                    device_idx, application.opencl.tmp.num_devices[platform_idx],
                                    platform_idx);
                            exit(STATION_APP_ERROR_ARGUMENTS);
                        }

                        application.opencl.contexts.context_info[i].device_ids[j++] =
                            application.opencl.tmp.device_list[platform_idx][device_idx];
                    }

                    chr >>= 1;
                }
            }
        }

        // Create OpenCL context
        cl_int ret;

        cl_context_properties context_properties[] = {CL_CONTEXT_PLATFORM,
            (cl_context_properties)application.opencl.contexts.context_info[i].platform_id, 0};

        application.opencl.contexts.contexts[i] = clCreateContext(context_properties,
                application.opencl.contexts.context_info[i].num_devices,
                application.opencl.contexts.context_info[i].device_ids,
                NULL, (void*)NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            ERROR_("couldn't create OpenCL context for <" COLOR_NUMBER "%s" COLOR_RESET ">",
                    application.args.cl_context_arg[i]);
            exit(STATION_APP_ERROR_OPENCL);
        }
    }
}

static void free_opencl_tmp_arrays(void)
{
    if (application.opencl.tmp.device_list != NULL)
        for (cl_uint platform_idx = 0; platform_idx <
                application.opencl.tmp.num_platforms; platform_idx++)
            free(application.opencl.tmp.device_list[platform_idx]);

    free(application.opencl.tmp.device_list);
    free(application.opencl.tmp.num_devices);
    free(application.opencl.tmp.platform_list);

    application.opencl.tmp.num_platforms = 0;
    application.opencl.tmp.platform_list = NULL;
    application.opencl.tmp.num_devices = NULL;
    application.opencl.tmp.device_list = NULL;
}

static void exit_release_opencl_contexts(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    exit_release_opencl_contexts_quick();

    if (application.opencl.contexts.contexts != NULL)
        free(application.opencl.contexts.contexts);

    if (application.opencl.contexts.context_info != NULL)
    {
        for (uint32_t i = 0; i < application.opencl.contexts.num_contexts; i++)
            free(application.opencl.contexts.context_info[i].device_ids);

        free(application.opencl.contexts.context_info);
    }

    application.opencl.contexts.num_contexts = 0;

    free_opencl_tmp_arrays();
}

static void exit_release_opencl_contexts_quick(void)
{
    EXIT_ASSERT_MAIN_THREAD();

    if (application.opencl.contexts.contexts != NULL)
    {
        for (uint32_t i = 0; i < application.opencl.contexts.num_contexts; i++)
            if (application.opencl.contexts.contexts[i] != NULL)
                clReleaseContext(application.opencl.contexts.contexts[i]);
    }
}

#endif

