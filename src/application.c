#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "application_args.h"

#include <station/plugin.typ.h>
#include <station/plugin.def.h>

#include <station/fsm.fun.h>
#include <station/fsm.def.h>

#include <station/sdl.typ.h>

#include <station/signal.fun.h>

int main(int argc, char *argv[])
{
    fprintf(stderr, " ]                                     [\n");
    fprintf(stderr, " ]         Station application         [\n");
    fprintf(stderr, " ] Copyright (C) 2024 by Ivan Podmazov [\n");
    fprintf(stderr, " ]                                     [\n\n");

    int code = 1;

    /////////////////////
    // Parse arguments //
    /////////////////////

    struct gengetopt_args_info args;
    args_parser_init(&args);

#define CLEANUP free_args

    int plugin_argc = 0;
    char **plugin_argv = argv + argc;
    {
        // Parse application arguments
        int app_argc = argc;

        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "--") == 0)
            {
                app_argc = i;
                plugin_argc = argc - i;
                plugin_argv = argv + i;
                break;
            }
        }

        if (args_parser(app_argc, argv, &args) != 0)
        {
            fprintf(stderr, "\nError: couldn't parse application arguments.\n");
            goto CLEANUP;
        }

        // Parse configuration files
        struct args_parser_params params;
        args_parser_params_init(&params);

        params.override = 0;
        params.initialize = 0;
        params.check_required = 1;
        params.check_ambiguity = 0;
        params.print_errors = 1;

        for (unsigned i = 0; i < args.argfile_given; i++)
        {
            if (args_parser_config_file(args.argfile_arg[i], &args, &params) != 0)
            {
                fprintf(stderr, "\nError: couldn't parse application arguments file '%s'.\n", args.argfile_arg[i]);
                goto CLEANUP;
            }
        }
    }

    if (!args.threads_given)
        args.threads_arg = 0;

    if (!args.window_width_given)
        args.window_width_arg = 0;

    if (!args.window_height_given)
        args.window_height_arg = 0;

    ////////////////////////////////////////////////
    // Check correctness of application arguments //
    ////////////////////////////////////////////////

    if (!args.help_given)
    {
        if (args.inputs_num > 1)
        {
            fprintf(stderr, "\nError: cannot process more than one plugin file.\n");
            goto CLEANUP;
        }
        else if (args.inputs_num == 0)
        {
            code = 0;
            goto CLEANUP;
        }

        if (args.threads_arg < 0)
        {
            fprintf(stderr, "\nError: number of threads cannot be negative.\n");
            goto CLEANUP;
        }

        if (args.window_width_arg < 0)
        {
            fprintf(stderr, "\nError: window width cannot be negative.\n");
            goto CLEANUP;
        }
        else if (args.window_height_arg < 0)
        {
            fprintf(stderr, "\nError: window height cannot be negative.\n");
            goto CLEANUP;
        }
    }
    else
    {
        if (args.inputs_num > 1)
        {
            fprintf(stderr, "\nError: cannot process more than one plugin file.\n");
            goto CLEANUP;
        }
        else if (args.inputs_num == 0)
        {
            args_parser_print_help();
            code = 0;
            goto CLEANUP;
        }
    }

    /////////////////////////////////
    // Configure signal management //
    /////////////////////////////////

    if (args.SIGHUP_given)
    {
        if (args.SIGHUP_arg == SIGHUP_arg_watch)
            station_signal_handler_watch_SIGHUP();
        else if (args.SIGHUP_arg == SIGHUP_arg_ignore)
            station_signal_handler_ignore_SIGHUP();
    }

    if (args.SIGINT_given)
    {
        if (args.SIGINT_arg == SIGINT_arg_watch)
            station_signal_handler_watch_SIGINT();
        else if (args.SIGINT_arg == SIGINT_arg_ignore)
            station_signal_handler_ignore_SIGINT();
    }

    if (args.SIGQUIT_given)
    {
        if (args.SIGQUIT_arg == SIGQUIT_arg_watch)
            station_signal_handler_watch_SIGQUIT();
        else if (args.SIGQUIT_arg == SIGQUIT_arg_ignore)
            station_signal_handler_ignore_SIGQUIT();
    }

    if (args.SIGUSR1_given)
    {
        if (args.SIGUSR1_arg == SIGUSR1_arg_watch)
            station_signal_handler_watch_SIGUSR1();
        else if (args.SIGUSR1_arg == SIGUSR1_arg_ignore)
            station_signal_handler_ignore_SIGUSR1();
    }

    if (args.SIGUSR2_given)
    {
        if (args.SIGUSR2_arg == SIGUSR2_arg_watch)
            station_signal_handler_watch_SIGUSR2();
        else if (args.SIGUSR2_arg == SIGUSR2_arg_ignore)
            station_signal_handler_ignore_SIGUSR2();
    }

    if (args.SIGALRM_given)
    {
        if (args.SIGALRM_arg == SIGALRM_arg_watch)
            station_signal_handler_watch_SIGALRM();
        else if (args.SIGALRM_arg == SIGALRM_arg_ignore)
            station_signal_handler_ignore_SIGALRM();
    }

    if (args.SIGTERM_given)
    {
        if (args.SIGTERM_arg == SIGTERM_arg_watch)
            station_signal_handler_watch_SIGTERM();
        else if (args.SIGTERM_arg == SIGTERM_arg_ignore)
            station_signal_handler_ignore_SIGTERM();
    }

    if (args.SIGTSTP_given)
    {
        if (args.SIGTSTP_arg == SIGTSTP_arg_watch)
            station_signal_handler_watch_SIGTSTP();
        else if (args.SIGTSTP_arg == SIGTSTP_arg_ignore)
            station_signal_handler_ignore_SIGTSTP();
    }

    if (args.SIGTTIN_given)
    {
        if (args.SIGTTIN_arg == SIGTTIN_arg_watch)
            station_signal_handler_watch_SIGTTIN();
        else if (args.SIGTTIN_arg == SIGTTIN_arg_ignore)
            station_signal_handler_ignore_SIGTTIN();
    }

    if (args.SIGTTOU_given)
    {
        if (args.SIGTTOU_arg == SIGTTOU_arg_watch)
            station_signal_handler_watch_SIGTTOU();
        else if (args.SIGTTOU_arg == SIGTTOU_arg_ignore)
            station_signal_handler_ignore_SIGTTOU();
    }

    if (args.SIGWINCH_given)
    {
        if (args.SIGWINCH_arg == SIGWINCH_arg_watch)
            station_signal_handler_watch_SIGWINCH();
        else if (args.SIGWINCH_arg == SIGWINCH_arg_ignore)
            station_signal_handler_ignore_SIGWINCH();
    }

    /////////////////////
    // Print arguments //
    /////////////////////

    if (args.verbose_given)
    {
        fprintf(stderr, "App version: %u\n\n", STATION_PLUGIN_VERSION);

        fprintf(stderr, "Plugin: %s\n", args.inputs[0]);
        if (plugin_argc > 0)
        {
            fprintf(stderr, "  with %i arguments:\n", plugin_argc);
            for (int i = 0; i < plugin_argc; i++)
                fprintf(stderr, "    %i: %s\n", i, plugin_argv[i]);
        }
        else
            fprintf(stderr, "  with no arguments\n");

        fprintf(stderr, "\n");

        if (!args.help_given)
        {
            if (args.no_sdl_given)
                fprintf(stderr, "Window: none\n");
            else
            {
                fprintf(stderr, "Window: ");

                if (args.window_width_arg > 0)
                    fprintf(stderr, "%i", args.window_width_arg);
                else
                    fprintf(stderr, "(texture width)");

                fprintf(stderr, "x");

                if (args.window_height_arg > 0)
                    fprintf(stderr, "%i", args.window_height_arg);
                else
                    fprintf(stderr, "(texture height)");

                fprintf(stderr, "\n");
            }

            fprintf(stderr, "\n");
        }
    }

    //////////////////////
    // Load plugin file //
    //////////////////////

    void *plugin = dlopen(args.inputs[0], RTLD_NOW | RTLD_LOCAL);
    if (plugin == NULL)
    {
        fprintf(stderr, "\nError: couldn't load plugin '%s'.\n", args.inputs[0]);
        goto CLEANUP;
    }

#undef CLEANUP
#define CLEANUP unload_plugin

    /////////////////////////
    // Check plugin format //
    /////////////////////////

#define STRINGIFY(obj) #obj

    {
        station_plugin_format_t *plugin_format = dlsym(plugin, STRINGIFY(STATION_PLUGIN_FORMAT_OBJECT));
        if (plugin_format == NULL)
        {
            fprintf(stderr, "\nError: couldn't obtain plugin format.\n");
            goto CLEANUP;
        }

        if (plugin_format->signature != STATION_PLUGIN_SIGNATURE)
        {
            fprintf(stderr, "\nError: plugin signature (0x%X) is wrong (must be 0x%X).\n",
                    plugin_format->signature, STATION_PLUGIN_SIGNATURE);
            goto CLEANUP;
        }

        if (plugin_format->version != STATION_PLUGIN_VERSION)
        {
            fprintf(stderr, "\nError: plugin version (%u) is different from application version (%u).\n",
                    plugin_format->version, STATION_PLUGIN_VERSION);
            goto CLEANUP;
        }
    }

    //////////////////////////
    // Obtain plugin vtable //
    //////////////////////////

    station_plugin_vtable_t *plugin_vtable = dlsym(plugin, STRINGIFY(STATION_PLUGIN_VTABLE_OBJECT));
    if (plugin_vtable == NULL)
    {
        fprintf(stderr, "\nError: couldn't obtain plugin vtable.\n");
        goto CLEANUP;
    }

    if ((plugin_vtable->help_fn == NULL) ||
            (plugin_vtable->init_fn == NULL) ||
            (plugin_vtable->final_fn == NULL))
    {
        fprintf(stderr, "\nError: plugin vtable contains NULL pointers.\n");
        goto CLEANUP;
    }

    //////////////////////////////////
    // Display help if in help mode //
    //////////////////////////////////

    if (args.help_given)
    {
        if (args.verbose_given)
        {
            fprintf(stderr, "<<< Beginning of plugin help >>>\n");
            fprintf(stderr, "===============================================================================\n\n");
        }

        plugin_vtable->help_fn(plugin_argc, plugin_argv);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin help >>>\n");
        }

        code = 0;
        goto CLEANUP;
    }

    ///////////////////////
    // Initialize plugin //
    ///////////////////////

    void *plugin_resources = NULL;

    station_state_t initial_state = {0};
    station_threads_number_t num_threads = args.threads_arg;

    station_sdl_properties_t sdl_properties = {
        .window_width = args.window_width_arg,
        .window_height = args.window_height_arg,
        .window_shown = args.window_shown_given,
        .window_resizable = args.window_resizable_given,
        .window_title = args.window_title_arg,
    };

    station_sdl_context_t sdl_context = {0};

    {
        if (args.verbose_given)
        {
            fprintf(stderr, "<<< Beginning of plugin initialization >>>\n");
            fprintf(stderr, "===============================================================================\n\n");
        }

        plugin_resources = plugin_vtable->init_fn(&initial_state, &num_threads,
                args.no_sdl_given ? (station_sdl_properties_t*)NULL : &sdl_properties,
                args.no_sdl_given ? (station_sdl_context_t*)NULL : &sdl_context,
                plugin_argc, plugin_argv);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin initialization >>>\n");
        }

        if (!args.no_sdl_given)
        {
            if (sdl_properties.window_title == NULL)
                sdl_properties.window_title = args.inputs[0];
        }

        if (args.verbose_given)
        {
            fprintf(stderr, "\n");
            fprintf(stderr, "Threads: %u\n", num_threads);
            if (!args.no_sdl_given)
                fprintf(stderr, "SDL texture: %ux%u\n", sdl_properties.texture_width, sdl_properties.texture_height);
            fprintf(stderr, "\n");
        }
    }

    //////////////////////////////////////
    // Execute the finite state machine //
    //////////////////////////////////////

    {
        if (args.verbose_given)
        {
            fprintf(stderr, "<<< Beginning of plugin execution >>>\n");
            fprintf(stderr, "===============================================================================\n\n");
        }

        uint8_t fsm_code;

        if (args.no_sdl_given)
            fsm_code = station_finite_state_machine(initial_state, num_threads);
        else
            fsm_code = station_finite_state_machine_sdl(initial_state, num_threads, &sdl_properties, &sdl_context);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin execution >>>\n");
        }

        if (fsm_code != STATION_FSM_EXEC_SUCCESS)
        {
            fprintf(stderr, "\nError: Finite state machine returned code %u.\n", fsm_code);
            fprintf(stderr, "\n");
        }
    }

    /////////////////////
    // Finalize plugin //
    /////////////////////

    {
        if (args.verbose_given)
        {
            fprintf(stderr, "<<< Beginning of plugin finalization >>>\n");
            fprintf(stderr, "===============================================================================\n\n");
        }

        code = plugin_vtable->final_fn(plugin_resources);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin finalization >>>\n");
        }
    }

    ///////////////////////
    // Release resources //
    ///////////////////////

#undef CLEANUP

unload_plugin:
    dlclose(plugin);

free_args:
    args_parser_free(&args);

    return code;
}

