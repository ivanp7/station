#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dlfcn.h>

#ifdef STATION_IS_OPENCL_SUPPORTED
#  include <CL/cl.h>
#endif

#include <station/plugin.typ.h>
#include <station/plugin.def.h>
#include <station/signal.fun.h>
#include <station/signal.typ.h>
#include <station/sdl.typ.h>
#include <station/opencl.typ.h>
#include <station/fsm.fun.h>
#include <station/fsm.def.h>

#include "application_args.h"


#define CODE_OK 0
#define CODE_ERROR_GENERAL 1
#define CODE_ERROR_ARGUMENTS 2
#define CODE_ERROR_SIGNAL 3
#define CODE_ERROR_PLUGIN 4
#define CODE_ERROR_OPENCL 5
#define CODE_ERROR_FSM 6
#define CODE_ERROR_FSM_SDL 7
#define CODE_ERROR_MALLOC 8
#define CODE_ERROR_USER 16


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
#define COLOR_VERSION COLOR_FG_BRI_BLUE
#define COLOR_OUTPUT_SEGMENT COLOR_FG_BRI_BLACK
#define COLOR_ERROR COLOR_FG_BRI_RED

#define OUTPUT_SEGMENT_SEPARATOR \
    "==============================================================================="

#define PRINT(msg) fprintf(stderr, msg)
#define PRINT_(msg, ...) fprintf(stderr, msg, __VA_ARGS__)

#define ERROR(msg) fprintf(stderr, "\n" COLOR_ERROR "Error" COLOR_RESET \
        ": " msg ".\n")
#define ERROR_(msg, ...) fprintf(stderr, "\n" COLOR_ERROR "Error" COLOR_RESET \
        ": " msg ".\n", __VA_ARGS__)


#define STRINGIFY(obj) STRING_OF(obj)
#define STRING_OF(obj) #obj


static struct {
    struct gengetopt_args_info args;

    struct {
        int argc;
        char **argv;

        void *handle;

        station_plugin_format_t *format;
        station_plugin_vtable_t *vtable;

        void *resources;
    } plugin;

    struct {
        station_signal_set_t set, *set_ptr;
        struct station_signal_management_context *management_context;
    } signal;

    struct {
        station_sdl_properties_t properties, *properties_ptr;
        station_sdl_context_t context, *context_ptr;
    } sdl;

    struct {
        bool not_needed;
        station_opencl_context_t context, *context_ptr;
    } opencl;

    struct {
        station_state_t initial_state;
        void *data;
        station_threads_number_t num_threads;
    } fsm;
} application;


static int with_args(void);
static int with_plugin(void);
static int with_plugin_resources(void);
static int with_signals(void);
static int with_opencl_contexts(void);

#ifdef STATION_IS_OPENCL_SUPPORTED
static int create_opencl_contexts(
        cl_uint num_platforms, cl_platform_id *platform_list,
        cl_uint *num_devices, cl_device_id **device_list);
#endif


int main(int argc, char *argv[])
{
    ///////////////////////////////////
    // Initialize supported features //
    ///////////////////////////////////

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    application.signal.set_ptr = &application.signal.set;
#endif

#ifdef STATION_IS_SDL_SUPPORTED
    application.sdl.properties_ptr = &application.sdl.properties;
    application.sdl.context_ptr = &application.sdl.context;
#endif

#ifdef STATION_IS_OPENCL_SUPPORTED
    application.opencl.context_ptr = &application.opencl.context;
#endif

    //////////////////////////////////////////////////
    // Split application and plugin arguments apart //
    //////////////////////////////////////////////////

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

    if (args_parser(app_argc, argv, &application.args) != 0)
    {
        ERROR("couldn't parse application arguments");
        args_parser_free(&application.args);
        return CODE_ERROR_ARGUMENTS;
    }

    ///////////////////////////////
    // Parse configuration files //
    ///////////////////////////////

    {
        struct args_parser_params params;
        args_parser_params_init(&params);

        params.override = 0;
        params.initialize = 0;
        params.check_required = 1;
        params.check_ambiguity = 0;
        params.print_errors = 1;

        for (unsigned i = 0; i < application.args.conf_given; i++)
        {
            if (args_parser_config_file(application.args.conf_arg[i],
                        &application.args, &params) != 0)
            {
                ERROR_("couldn't parse arguments from file "
                        COLOR_STRING "%s" COLOR_RESET, application.args.conf_arg[i]);
                args_parser_free(&application.args);
                return CODE_ERROR_ARGUMENTS;
            }
        }
    }

    ////////////////////////////////////
    // Continue with parsed arguments //
    ////////////////////////////////////

    int code = with_args();

    ///////////////////////////////////
    // Release application arguments //
    ///////////////////////////////////

    bool verbose = application.args.verbose_given;
    args_parser_free(&application.args);

    /////////////////////////
    // Print final message //
    /////////////////////////

    if (verbose)
        PRINT("\nApplication terminated normally.\n");

    return code;
}


static int with_args(void)
{
    //////////////////////
    // Display the logo //
    //////////////////////

    if (!application.args.no_logo_given)
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
                                                         (C) 2024\n\
\n\n" COLOR_RESET);

    /////////////////////////////////////////////////////
    // Display application version and feature support //
    /////////////////////////////////////////////////////

    if (application.args.verbose_given)
    {
        PRINT_("Version : " COLOR_VERSION "%u" COLOR_RESET "\n", STATION_PLUGIN_VERSION);

        PRINT("Signals : ");
#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
        PRINT(COLOR_FLAG_ON "supported");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "\n");

        PRINT("SDL     : ");
#ifdef STATION_IS_SDL_SUPPORTED
        PRINT(COLOR_FLAG_ON "supported");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "\n");

        PRINT("OpenCL  : ");
#ifdef STATION_IS_OPENCL_SUPPORTED
        PRINT(COLOR_FLAG_ON "supported");
#else
        PRINT(COLOR_FLAG_OFF "not supported");
#endif
        PRINT(COLOR_RESET "\n");

        PRINT("\n");
    }

    ////////////////////////////////////////
    // Assign default values to arguments //
    ////////////////////////////////////////

    if (!application.args.threads_given)
        application.args.threads_arg = 0;

#ifdef STATION_IS_SDL_SUPPORTED
    if (!application.args.window_width_given)
        application.args.window_width_arg = 0;

    if (!application.args.window_height_given)
        application.args.window_height_arg = 0;
#endif

    ////////////////////////////////////////////////
    // Check correctness of application arguments //
    ////////////////////////////////////////////////

    if (application.args.cl_list_given)
    {
        if (application.args.inputs_num != 0)
        {
            ERROR("processing a plugin file while displaying list of OpenCL platforms/devices is not supported");
            return CODE_ERROR_ARGUMENTS;
        }

        if (application.args.help_given)
        {
            ERROR("displaying list of OpenCL platforms/devices is mutually exclusive with help mode");
            return CODE_ERROR_ARGUMENTS;
        }
    }
    else if (application.args.help_given)
    {
        if (application.args.inputs_num > 1)
        {
            ERROR("cannot process more than one plugin file");
            return CODE_ERROR_ARGUMENTS;
        }
        else if (application.args.inputs_num == 0)
        {
            args_parser_print_help();
            return CODE_OK;
        }
    }
    else
    {
        if (application.args.inputs_num > 1)
        {
            ERROR("cannot process more than one plugin file");
            return CODE_ERROR_ARGUMENTS;
        }

        if (application.args.threads_arg < 0)
        {
            ERROR("number of threads cannot be negative");
            return CODE_ERROR_ARGUMENTS;
        }

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
                return CODE_ERROR_ARGUMENTS;
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
                return CODE_ERROR_ARGUMENTS;
            }

            if (device_mask != NULL)
            {
                size_t device_mask_len = strlen(device_mask);

                if (device_mask_len == 0)
                {
                    ERROR_("OpenCL device mask cannot be empty (context argument ["
                            COLOR_NUMBER "%u" COLOR_RESET "])", i);
                    return CODE_ERROR_ARGUMENTS;
                }
                else if (strspn(device_mask, "0123456789ABCDEFabcdef") != device_mask_len)
                {
                    ERROR_("OpenCL device mask contains invalid characters: "
                            COLOR_NUMBER "%s" COLOR_RESET " (context argument ["
                            COLOR_NUMBER "%u" COLOR_RESET "])", device_mask, i);
                    return CODE_ERROR_ARGUMENTS;
                }
                else if (strspn(device_mask, "0") == device_mask_len)
                {
                    ERROR_("OpenCL device mask cannot be zero (context argument ["
                            COLOR_NUMBER "%u" COLOR_RESET "])", i);
                    return CODE_ERROR_ARGUMENTS;
                }
            }
        }
#endif

#ifdef STATION_IS_SDL_SUPPORTED
        if (application.args.window_width_given && (application.args.window_width_arg <= 0))
        {
            ERROR("window width must be positive");
            return CODE_ERROR_ARGUMENTS;
        }

        if (application.args.window_height_given && (application.args.window_height_arg <= 0))
        {
            ERROR("window height must be positive");
            return CODE_ERROR_ARGUMENTS;
        }
#endif
    }

    //////////////////////////////////////////////
    // Display list of OpenCL platforms/devices //
    //////////////////////////////////////////////

#ifdef STATION_IS_OPENCL_SUPPORTED
    if (application.args.cl_list_given)
    {
        cl_int ret;

        cl_uint num_platforms;

        ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &num_platforms);
        if (ret != CL_SUCCESS)
        {
            ERROR("couldn't obtain number of OpenCL platforms");
            return CODE_ERROR_OPENCL;
        }

        PRINT_("Number of OpenCL platforms: " COLOR_NUMBER "%u" COLOR_RESET "\n", num_platforms);

        cl_platform_id *platform_list = malloc(sizeof(cl_platform_id) * num_platforms);
        if (platform_list == NULL)
        {
            ERROR("couldn't allocate OpenCL platform list");
            return CODE_ERROR_MALLOC;
        }

        ret = clGetPlatformIDs(num_platforms, platform_list, (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            ERROR("couldn't obtain OpenCL platform list");
            free(platform_list);
            return CODE_ERROR_OPENCL;
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
                    return CODE_ERROR_OPENCL;
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
                    return CODE_ERROR_OPENCL;
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
                        return CODE_ERROR_MALLOC;
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
                    return CODE_ERROR_OPENCL;
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
                        return CODE_ERROR_OPENCL;
                    }

                    PRINT_("    #" COLOR_NUMBER "%x" COLOR_RESET ": "
                            COLOR_STRING "%s" COLOR_RESET "\n", device_idx, name);
                }
            }
        }

        free(platform_list);
        free(device_list);

        return CODE_OK;
    }
#endif

    ////////////////////////////////////////
    // Exit if no plugin file is provided //
    ////////////////////////////////////////

    if (application.args.inputs_num == 0)
    {
        if (application.args.verbose_given)
            PRINT("Plugin file is not specified, exiting.\n");

        return CODE_OK;
    }

    ///////////////////////
    // Display arguments //
    ///////////////////////

    if (application.args.verbose_given)
    {
        PRINT_("Plugin: " COLOR_STRING "%s" COLOR_RESET "\n", application.args.inputs[0]);
        if (application.plugin.argc > 0)
        {
            PRINT_("  with " COLOR_NUMBER "%i" COLOR_RESET " arguments:\n",
                    application.plugin.argc);
            for (int i = 0; i < application.plugin.argc; i++)
                PRINT_("    [" COLOR_NUMBER "%i" COLOR_RESET "]: "
                        COLOR_STRING "%s" COLOR_RESET "\n", i, application.plugin.argv[i]);
        }
        else
            PRINT("  with no arguments\n");

        PRINT("\n");
    }

    //////////////////////
    // Load plugin file //
    //////////////////////

    application.plugin.handle = dlopen(application.args.inputs[0], RTLD_NOW | RTLD_LOCAL);
    if (application.plugin.handle == NULL)
    {
        ERROR_("couldn't load plugin " COLOR_STRING "%s" COLOR_RESET, application.args.inputs[0]);
        return CODE_ERROR_PLUGIN;
    }

    /////////////////////////////////
    // Continue with loaded plugin //
    /////////////////////////////////

    int code = with_plugin();

    ////////////////////////
    // Unload plugin file //
    ////////////////////////

    dlclose(application.plugin.handle);

    return code;
}


static int with_plugin(void)
{
    /////////////////////////
    // Check plugin format //
    /////////////////////////

    {
        application.plugin.format = dlsym(application.plugin.handle,
                STRINGIFY(STATION_PLUGIN_FORMAT_OBJECT));
        if (application.plugin.format == NULL)
        {
            ERROR("couldn't obtain plugin format");
            return CODE_ERROR_PLUGIN;
        }

        if (application.plugin.format->signature != STATION_PLUGIN_SIGNATURE)
        {
            ERROR_("plugin signature (" COLOR_VERSION "0x%X" COLOR_RESET
                    ") is wrong (must be " COLOR_VERSION "0x%X" COLOR_RESET ")",
                    application.plugin.format->signature, STATION_PLUGIN_SIGNATURE);
            return CODE_ERROR_PLUGIN;
        }

        if (application.plugin.format->version != STATION_PLUGIN_VERSION)
        {
            ERROR_("plugin version (" COLOR_VERSION "%u" COLOR_RESET
                    ") is different from application version (" COLOR_VERSION "%u" COLOR_RESET ")",
                    application.plugin.format->version, STATION_PLUGIN_VERSION);
            return CODE_ERROR_PLUGIN;
        }
    }

    //////////////////////////
    // Obtain plugin vtable //
    //////////////////////////

    application.plugin.vtable = dlsym(application.plugin.handle,
            STRINGIFY(STATION_PLUGIN_VTABLE_OBJECT));
    if (application.plugin.vtable == NULL)
    {
        ERROR("couldn't obtain plugin vtable");
        return CODE_ERROR_PLUGIN;
    }

    if ((application.plugin.vtable->help_fn == NULL) ||
            (application.plugin.vtable->init_fn == NULL) ||
            (application.plugin.vtable->final_fn == NULL))
    {
        ERROR("plugin vtable contains NULL pointers");
        return CODE_ERROR_PLUGIN;
    }

    //////////////////////////////////
    // Display help if in help mode //
    //////////////////////////////////

    if (application.args.help_given)
    {
        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "<<< Beginning of plugin help >>>\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n\n" COLOR_RESET);
        }

        int code = application.plugin.vtable->help_fn(application.plugin.argc, application.plugin.argv);

        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
            PRINT("<<< End of plugin help >>>\n" COLOR_RESET);
        }

        return code;
    }

    {
        //////////////////////////////////////////////////////////
        // Prepare arguments for plugin initialization function //
        //////////////////////////////////////////////////////////

        if (application.signal.set_ptr != NULL)
        {
#define WATCH_SIGNAL(signame)                       \
            if (application.args.signame##_given)   \
                application.signal.set.signal_##signame = true;

            WATCH_SIGNAL(SIGHUP)
            WATCH_SIGNAL(SIGINT)
            WATCH_SIGNAL(SIGQUIT)
            WATCH_SIGNAL(SIGUSR1)
            WATCH_SIGNAL(SIGUSR2)
            WATCH_SIGNAL(SIGALRM)
            WATCH_SIGNAL(SIGTERM)
            WATCH_SIGNAL(SIGTSTP)
            WATCH_SIGNAL(SIGTTIN)
            WATCH_SIGNAL(SIGTTOU)
            WATCH_SIGNAL(SIGWINCH)

#undef WATCH_SIGNAL
        }

        if (application.args.no_sdl_given)
            application.sdl.properties_ptr = NULL;

        if (application.sdl.properties_ptr != NULL)
        {
            application.sdl.properties.window_width = application.args.window_width_arg;
            application.sdl.properties.window_height = application.args.window_height_arg;
            application.sdl.properties.window_resizable = application.args.window_resizable_given;
            application.sdl.properties.window_shown = application.args.window_shown_given;
            application.sdl.properties.window_title = application.args.window_title_arg;
        }

        station_plugin_init_func_args_t plugin_init_func_args = {
            .fsm_num_threads = application.args.threads_arg,
            .signals = application.signal.set_ptr,
            .sdl_properties = application.sdl.properties_ptr,
            .future_sdl_context = application.sdl.context_ptr,
            .future_opencl_context = application.opencl.context_ptr,
        };

        ///////////////////////
        // Initialize plugin //
        ///////////////////////

        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "<<< Beginning of plugin initialization >>>\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n\n" COLOR_RESET);
        }

        int init_code = application.plugin.vtable->init_fn(&plugin_init_func_args,
                application.plugin.argc, application.plugin.argv);

        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
            PRINT("<<< End of plugin initialization >>>\n" COLOR_RESET);
        }

        if (init_code != 0)
            return CODE_ERROR_USER + init_code;

        ///////////////////////////////////////////////////////
        // Configure the application as the plugin specified //
        ///////////////////////////////////////////////////////

        application.plugin.resources = plugin_init_func_args.plugin_resources;

        application.fsm.initial_state = plugin_init_func_args.fsm_initial_state;
        application.fsm.data = plugin_init_func_args.fsm_data;
        application.fsm.num_threads = plugin_init_func_args.fsm_num_threads;

        if (plugin_init_func_args.signals_not_needed)
            application.signal.set_ptr = NULL;

        if (plugin_init_func_args.sdl_not_needed)
            application.sdl.properties_ptr = NULL;

        if (plugin_init_func_args.opencl_not_needed)
            application.opencl.not_needed = true;

        /////////////////////////////////////////
        // Assign default values to properties //
        /////////////////////////////////////////

        if (application.sdl.properties_ptr != NULL)
        {
            if (application.sdl.properties.window_title == NULL)
                application.sdl.properties.window_title = application.args.inputs[0];
        }
    }

    ///////////////////////////////////////
    // Display application configuration //
    ///////////////////////////////////////

    if (application.args.verbose_given)
    {
        PRINT("\n");

        PRINT_("Threads : " COLOR_NUMBER "%u" COLOR_RESET "\n", application.fsm.num_threads);

        if (application.signal.set_ptr != NULL)
        {
            PRINT("Signals :");

#define PRINT_SIGNAL(signame)                               \
            if (application.signal.set.signal_##signame)    \
                PRINT(" " COLOR_SIGNAL #signame COLOR_RESET);

            PRINT_SIGNAL(SIGHUP)
            PRINT_SIGNAL(SIGINT)
            PRINT_SIGNAL(SIGQUIT)
            PRINT_SIGNAL(SIGUSR1)
            PRINT_SIGNAL(SIGUSR2)
            PRINT_SIGNAL(SIGALRM)
            PRINT_SIGNAL(SIGTERM)
            PRINT_SIGNAL(SIGTSTP)
            PRINT_SIGNAL(SIGTTIN)
            PRINT_SIGNAL(SIGTTOU)
            PRINT_SIGNAL(SIGWINCH)

#undef PRINT_SIGNAL

            PRINT("\n");
        }

        if (application.sdl.properties_ptr != NULL)
        {
            PRINT_("[SDL] Texture : " COLOR_NUMBER "%u" COLOR_RESET "x" COLOR_NUMBER "%u" COLOR_RESET "\n",
                    application.sdl.properties.texture_width,
                    application.sdl.properties.texture_height);

            PRINT("[SDL] Window  : " COLOR_NUMBER);

            if (application.sdl.properties.window_width > 0)
                PRINT_("%u", application.sdl.properties.window_width);
            else
                PRINT_("{%u}", application.sdl.properties.texture_width);

            PRINT(COLOR_RESET "x" COLOR_NUMBER);

            if (application.sdl.properties.window_height > 0)
                PRINT_("%u", application.sdl.properties.window_height);
            else
                PRINT_("{%u}", application.sdl.properties.texture_height);

            PRINT_(COLOR_RESET ", %s" COLOR_RESET "\n", application.sdl.properties.window_resizable ?
                    COLOR_FLAG_ON "resizable" : COLOR_FLAG_OFF "not resizable");
        }

#ifdef STATION_IS_OPENCL_SUPPORTED
        if (!application.opencl.not_needed)
        {
            PRINT_("[OpenCL] Contexts: " COLOR_NUMBER "%u" COLOR_RESET "\n",
                    application.args.cl_context_given);

            if (application.args.cl_context_given)
            {
                for (unsigned i = 0; i < application.args.cl_context_given; i++)
                {
                    cl_uint platform_idx = strtoul(
                            application.args.cl_context_arg[i], (char**)NULL, 16);

                    PRINT_("  [" COLOR_NUMBER "%u" COLOR_RESET "]: "
                            "platform #" COLOR_NUMBER "%x" COLOR_RESET ", ", i, platform_idx);

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
        }
#endif

        PRINT("\n");
    }

    ///////////////////////////////////////
    // Continue with allocated resources //
    ///////////////////////////////////////

    int code = with_plugin_resources();

    /////////////////////
    // Finalize plugin //
    /////////////////////

    {
        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "<<< Beginning of plugin finalization >>>\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n\n" COLOR_RESET);
        }

        int final_code = application.plugin.vtable->final_fn(application.plugin.resources);

        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
            PRINT("<<< End of plugin finalization >>>\n" COLOR_RESET);
        }

        if (code == CODE_OK)
        {
            if (final_code != 0)
                code = CODE_ERROR_USER + final_code;
        }
    }

    return code;
}


static int with_plugin_resources(void)
{
    ////////////////////////////////////
    // Start signal management thread //
    ////////////////////////////////////

    if (application.signal.set_ptr != NULL)
    {
        bool thread_needed = false;

#define CHECK_SIGNAL(signame)                           \
        if (application.signal.set.signal_##signame)    \
            thread_needed = true;

        CHECK_SIGNAL(SIGHUP)
        CHECK_SIGNAL(SIGINT)
        CHECK_SIGNAL(SIGQUIT)
        CHECK_SIGNAL(SIGUSR1)
        CHECK_SIGNAL(SIGUSR2)
        CHECK_SIGNAL(SIGALRM)
        CHECK_SIGNAL(SIGTERM)
        CHECK_SIGNAL(SIGTSTP)
        CHECK_SIGNAL(SIGTTIN)
        CHECK_SIGNAL(SIGTTOU)
        CHECK_SIGNAL(SIGWINCH)

#undef CHECK_SIGNAL

        if (thread_needed)
        {
            application.signal.management_context = station_signal_management_thread_start(
                    &application.signal.set);

            if (application.signal.management_context == NULL)
            {
                ERROR("couldn't configure signal management");
                return CODE_ERROR_SIGNAL;
            }
        }
    }

    ///////////////////////////////////
    // Continue with watched signals //
    ///////////////////////////////////

    int code = with_signals();

    ///////////////////////////////////
    // Stop signal management thread //
    ///////////////////////////////////

#ifdef STATION_IS_SIGNAL_MANAGEMENT_SUPPORTED
    if (application.signal.management_context != NULL)
        station_signal_management_thread_stop(application.signal.management_context);
#endif

    return code;
}


static int with_signals(void)
{
    ////////////////////////////
    // Create OpenCL contexts //
    ////////////////////////////

#ifdef STATION_IS_OPENCL_SUPPORTED
    if (!application.opencl.not_needed)
    {
        int code = CODE_OK;

        cl_uint num_platforms = 0;
        cl_platform_id *platform_list = NULL;
        cl_uint *num_devices = NULL;
        cl_device_id **device_list = NULL;

        cl_int ret;

        // Obtain platform list
        ret = clGetPlatformIDs(0, (cl_platform_id*)NULL, &num_platforms);
        if (ret != CL_SUCCESS)
        {
            ERROR("couldn't obtain number of OpenCL platforms");
            code = CODE_ERROR_OPENCL;
            goto opencl_cleanup;
        }

        platform_list = malloc(sizeof(cl_platform_id) * num_platforms);
        if (platform_list == NULL)
        {
            ERROR("couldn't allocate OpenCL platform list");
            code = CODE_ERROR_MALLOC;
            goto opencl_cleanup;
        }

        ret = clGetPlatformIDs(num_platforms, platform_list, (cl_uint*)NULL);
        if (ret != CL_SUCCESS)
        {
            ERROR("couldn't obtain OpenCL platform list");
            code = CODE_ERROR_OPENCL;
            goto opencl_cleanup;
        }

        // Obtain sizes of device lists
        num_devices = malloc(sizeof(cl_uint) * num_platforms);
        if (num_devices == NULL)
        {
            ERROR("couldn't allocate list of numbers of OpenCL devices");
            code = CODE_ERROR_MALLOC;
            goto opencl_cleanup;
        }

        for (cl_uint platform_idx = 0; platform_idx < num_platforms; platform_idx++)
        {
            ret = clGetDeviceIDs(platform_list[platform_idx], CL_DEVICE_TYPE_DEFAULT,
                    0, (cl_device_id*)NULL, &num_devices[platform_idx]);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain number of OpenCL devices for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                code = CODE_ERROR_OPENCL;
                goto opencl_cleanup;
            }
        }

        // Obtain device lists
        device_list = malloc(sizeof(cl_device_id*) * num_platforms);
        if (device_list == NULL)
        {
            ERROR("couldn't allocate list of OpenCL device lists");
            code = CODE_ERROR_MALLOC;
            goto opencl_cleanup;
        }

        for (cl_uint platform_idx = 0; platform_idx < num_platforms; platform_idx++)
            device_list[platform_idx] = NULL;

        for (cl_uint platform_idx = 0; platform_idx < num_platforms; platform_idx++)
        {
            device_list[platform_idx] = malloc(sizeof(cl_device_id) * num_devices[platform_idx]);
            if (device_list[platform_idx] == NULL)
            {
                ERROR_("couldn't allocate OpenCL device list for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                code = CODE_ERROR_MALLOC;
                goto opencl_cleanup;
            }

            ret = clGetDeviceIDs(platform_list[platform_idx], CL_DEVICE_TYPE_DEFAULT,
                    num_devices[platform_idx], device_list[platform_idx], (cl_uint*)NULL);
            if (ret != CL_SUCCESS)
            {
                ERROR_("couldn't obtain OpenCL device list for platform #"
                        COLOR_NUMBER "%x" COLOR_RESET, platform_idx);
                code = CODE_ERROR_OPENCL;
                goto opencl_cleanup;
            }
        }

        // Create OpenCL contexts
        if (code == CODE_OK)
            code = create_opencl_contexts(num_platforms, platform_list, num_devices, device_list);

opencl_cleanup:
        // Free temporary arrays
        if (device_list != NULL)
        {
            for (cl_uint platform_idx = 0; platform_idx < num_platforms; platform_idx++)
                free(device_list[platform_idx]);
        }
        free(device_list);
        free(num_devices);
        free(platform_list);

        if (code != CODE_OK)
            return code;
    }
#endif

    ///////////////////////////////////////
    // Continue with initialized context //
    ///////////////////////////////////////

    int code = with_opencl_contexts();

    ////////////////////////////
    // Release OpenCL context //
    ////////////////////////////

#ifdef STATION_IS_OPENCL_SUPPORTED
    if (application.opencl.context.contexts != NULL)
    {
        for (uint32_t i = 0; i < application.opencl.context.num_platforms; i++)
            if (application.opencl.context.contexts[i] != NULL)
                clReleaseContext(application.opencl.context.contexts[i]);

        free(application.opencl.context.contexts);
    }

    if (application.opencl.context.platforms != NULL)
    {
        for (uint32_t i = 0; i < application.opencl.context.num_platforms; i++)
            free(application.opencl.context.platforms[i].device_ids);

        free(application.opencl.context.platforms);
    }
#endif

    return code;
}


static int with_opencl_contexts(void)
{
    //////////////////////////////////////
    // Execute the finite state machine //
    //////////////////////////////////////

    {
        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "<<< Beginning of plugin execution >>>\n");
            PRINT(OUTPUT_SEGMENT_SEPARATOR "\n\n" COLOR_RESET);
        }

        uint8_t fsm_code;

        if (application.sdl.properties_ptr == NULL)
            fsm_code = station_finite_state_machine(
                    application.fsm.initial_state, application.fsm.data, application.fsm.num_threads);
        else
            fsm_code = station_finite_state_machine_sdl(
                    application.fsm.initial_state, application.fsm.data, application.fsm.num_threads,
                    application.sdl.properties_ptr, application.sdl.context_ptr);

        if (application.args.verbose_given)
        {
            PRINT(COLOR_OUTPUT_SEGMENT "\n" OUTPUT_SEGMENT_SEPARATOR "\n");
            PRINT("<<< End of plugin execution >>>\n" COLOR_RESET);
        }

        if (fsm_code == STATION_FSM_EXEC_SUCCESS)
            return CODE_OK;
        else if (STATION_FSM_EXEC_IS_SDL_FAILED(fsm_code))
        {
            ERROR_("finite state machine function returned code "
                    COLOR_ERROR "%u" COLOR_RESET " (SDL failure)", fsm_code);
            PRINT("\n");
            return CODE_ERROR_FSM_SDL;
        }
        else
        {
            ERROR_("finite state machine function returned code " COLOR_ERROR "%u" COLOR_RESET, fsm_code);
            PRINT("\n");
            return CODE_ERROR_FSM;
        }
    }
}


#ifdef STATION_IS_OPENCL_SUPPORTED
static int create_opencl_contexts(
        cl_uint num_platforms, cl_platform_id *platform_list,
        cl_uint *num_devices, cl_device_id **device_list)
{
    application.opencl.context.num_platforms = application.args.cl_context_given;

    // Allocate array of contexts
    application.opencl.context.contexts = malloc(sizeof(cl_context) *
            application.opencl.context.num_platforms);
    if (application.opencl.context.contexts == NULL)
    {
        ERROR("couldn't allocate array of OpenCL platform contexts");
        return CODE_ERROR_MALLOC;
    }

    for (cl_uint i = 0; i < application.opencl.context.num_platforms; i++)
        application.opencl.context.contexts[i] = NULL;

    // Allocate array of platforms
    application.opencl.context.platforms = malloc(sizeof(*application.opencl.context.platforms) *
            application.opencl.context.num_platforms);
    if (application.opencl.context.platforms == NULL)
    {
        ERROR("couldn't allocate array of OpenCL platforms");
        return CODE_ERROR_MALLOC;
    }

    for (cl_uint i = 0; i < application.opencl.context.num_platforms; i++)
    {
        application.opencl.context.platforms[i].platform_id = NULL;
        application.opencl.context.platforms[i].device_ids = NULL;
        application.opencl.context.platforms[i].num_devices = 0;
    }

    // Create contexts
    for (cl_uint i = 0; i < application.opencl.context.num_platforms; i++)
    {
        // Extract platform index
        cl_uint platform_idx = strtoul(application.args.cl_context_arg[i], (char**)NULL, 16);
        if (platform_idx >= num_platforms)
        {
            ERROR_("OpenCL platform index #" COLOR_NUMBER "%x" COLOR_RESET
                    " is greater or equal to number of available platforms ("
                    COLOR_NUMBER "%u" COLOR_RESET ")",
                    platform_idx, num_platforms);
            return CODE_ERROR_ARGUMENTS;
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
        application.opencl.context.platforms[i].platform_id = platform_list[platform_idx];

        // Compute number of devices from device mask
        if (device_mask == NULL)
            application.opencl.context.platforms[i].num_devices = num_devices[platform_idx];
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

                application.opencl.context.platforms[i].num_devices +=
                    (int[16]){0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4}[chr];
            }

            if (application.opencl.context.platforms[i].num_devices > num_devices[platform_idx])
            {
                ERROR_("OpenCL device mask " COLOR_NUMBER "%s" COLOR_RESET
                        " enables more devices than available ("
                        COLOR_NUMBER "%u" COLOR_RESET ") on platform #"
                        COLOR_NUMBER "%x" COLOR_RESET,
                        device_mask, num_devices[platform_idx], platform_idx);
                return CODE_ERROR_ARGUMENTS;
            }
        }

        // Allocate array of devices
        application.opencl.context.platforms[i].device_ids = malloc(sizeof(cl_device_id) *
                application.opencl.context.platforms[i].num_devices);
        if (application.opencl.context.platforms[i].device_ids == NULL)
        {
            ERROR("couldn't allocate array of OpenCL devices");
            return CODE_ERROR_MALLOC;
        }

        // Copy device IDs
        if (device_mask == NULL)
        {
            for (cl_uint device_idx = 0; device_idx < num_devices[platform_idx]; device_idx++)
                application.opencl.context.platforms[i].device_ids[device_idx] = device_list[platform_idx][device_idx];
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
                        if (device_idx >= num_devices[platform_idx])
                        {
                            ERROR_("OpenCL device index #" COLOR_NUMBER "%x" COLOR_RESET
                                    " is greater or equal to number of available devices ("
                                    COLOR_NUMBER "%u" COLOR_RESET ") on platform #"
                                    COLOR_NUMBER "%x" COLOR_RESET,
                                    device_idx, num_devices[platform_idx], platform_idx);
                            return CODE_ERROR_ARGUMENTS;
                        }

                        application.opencl.context.platforms[i].device_ids[j++] = device_list[platform_idx][device_idx];
                    }

                    chr >>= 1;
                }
            }
        }

        // Create OpenCL context
        cl_int ret;

        cl_context_properties context_properties[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties)application.opencl.context.platforms[i].platform_id, 0};

        application.opencl.context.contexts[i] = clCreateContext(context_properties,
                application.opencl.context.platforms[i].num_devices, application.opencl.context.platforms[i].device_ids,
                NULL, (void*)NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            ERROR_("couldn't create OpenCL context for <" COLOR_NUMBER "%s" COLOR_RESET ">",
                    application.args.cl_context_arg[i]);
            return CODE_ERROR_OPENCL;
        }
    }

    return CODE_OK;
}
#endif

