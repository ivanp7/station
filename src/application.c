#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "application_args.h"

#include <station/app_plugin.typ.h>
#include <station/app_plugin.def.h>
#include <station/fsm.fun.h>
#include <station/sdl.typ.h>

#define STRINGIFY(obj) #obj

int main(int argc, const char *argv[])
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
    const char **plugin_argv = argv + argc;
    {
        // Parse application arguments
        int app_argc = argc;

        for (int i = 1; i < argc; i++)
            if (strcmp(argv[i], "--") == 0)
            {
                app_argc = i;
                plugin_argc = argc - i;
                plugin_argv = argv + i;
                break;
            }

        if (args_parser(app_argc, (char**)argv, &args) != 0)
        {
            fprintf(stderr, "\nError: couldn't parse application arguments.\n");
            goto CLEANUP;
        }

        // Parse configuration files
        struct args_parser_params params;
        args_parser_params_init(&params);

        params.override = false;
        params.initialize = false;
        params.check_required = true;
        params.check_ambiguity = false;
        params.print_errors = true;

        for (unsigned i = 0; i < args.argfile_given; i++)
            if (args_parser_config_file(args.argfile_arg[i], &args, &params) != 0)
            {
                fprintf(stderr, "\nError: couldn't parse application arguments file '%s'.\n", args.argfile_arg[i]);
                goto CLEANUP;
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
            if (args.no_window_given)
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

    station_plugin_context_t plugin_context = {
        .num_threads = args.threads_arg,
        .sdl_window_title = args.window_title_arg,
    };

    station_sdl_context_t sdl_context = {0};

    {
        if (args.verbose_given)
        {
            fprintf(stderr, "<<< Beginning of plugin initialization >>>\n");
            fprintf(stderr, "===============================================================================\n\n");
        }

        bool init_successful = plugin_vtable->init_fn(&plugin_context,
                args.no_window_given ? NULL : &sdl_context,
                plugin_argc, plugin_argv);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin initialization >>>\n");
        }

        if (!init_successful)
        {
            fprintf(stderr, "\nError: plugin initialization was not successful.\n");
            goto CLEANUP;
        }

        if (plugin_context.sdl_texture_width == 0)
        {
            fprintf(stderr, "\nError: plugin failed to set non-zero texture width.\n");
            goto CLEANUP;
        }
        else if (plugin_context.sdl_texture_height == 0)
        {
            fprintf(stderr, "\nError: plugin failed to set non-zero texture height.\n");
            goto CLEANUP;
        }

        if (plugin_context.sdl_window_title == NULL)
            plugin_context.sdl_window_title = args.inputs[0];

        if (args.verbose_given)
        {
            fprintf(stderr, "\n");
            fprintf(stderr, "Threads: %u\n", plugin_context.num_threads);
            if (!args.no_window_given)
                fprintf(stderr, "SDL texture: %ux%u\n", plugin_context.sdl_texture_width, plugin_context.sdl_texture_height);
            fprintf(stderr, "\n");
        }
    }

    //////////////////////////////////////
    // Execute the finite state machine //
    //////////////////////////////////////

    uint8_t fsm_error = 0;

    if (plugin_context.initial_state != NULL)
    {
        if (args.verbose_given)
        {
            fprintf(stderr, "<<< Beginning of plugin execution >>>\n");
            fprintf(stderr, "===============================================================================\n\n");
        }

        if (args.no_window_given)
            fsm_error = station_finite_state_machine(plugin_context.initial_state, plugin_context.num_threads);
        else
            fsm_error = station_finite_state_machine_sdl(plugin_context.initial_state, plugin_context.num_threads,
                    &sdl_context, plugin_context.sdl_init_flags,
                    plugin_context.sdl_texture_width, plugin_context.sdl_texture_height,
                    plugin_context.sdl_window_title, args.window_width_arg, args.window_height_arg);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin execution >>>\n");
        }

        if (fsm_error != 0)
        {
            fprintf(stderr, "\nError: Finite state machine returned error code %u.\n", fsm_error);
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

        bool exec_successful = plugin_vtable->final_fn(plugin_context.resources);

        if (args.verbose_given)
        {
            fprintf(stderr, "\n===============================================================================\n");
            fprintf(stderr, "<<< End of plugin finalization >>>\n");
        }

        if (!exec_successful)
        {
            fprintf(stderr, "\nError: plugin execution was not successful.\n");
            goto CLEANUP;
        }
    }

    if (fsm_error == 0)
        code = 0;

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

