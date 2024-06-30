# Station, a resource provider library/application

## Features of the library (`libstation`)

The library provides types, functions, macros, constants for the following things:

* finite-state machines as algorithm of execution (`fsm.*.h`);
* concurrent processing using threads (`concurrent.*.h`);
* signal management in multithreading environment (`signal.*.h`);
* quick & easy SDL windows for drawing (`sdl.*.h`);
* fonts support for drawing (`font.*.h`);
* other!

## Features of the application (`station-app`)

The application helps with the initialization and release of resources
for the user-written plugins, which don't need to bother to do it manually.

Besides the features of the library, the application is also capable
to create OpenCL contexts (`opencl.*.h`).

The application works as follows:

1. Command line arguments are parsed.

2. Plugin file is loaded.

3. Plugin configuration function is called.
This function allows the plugin to select and configure features provided by
the application, and also to parse command line arguments for the plugin.

4. Application resources are initialized by the application.

5. Plugin initialization function is called,
the application resources are passed to it by pointers.
The plugin prepares the initial finite state machine state, and creates its own resources.

6. The finite state machine is executed.

```c
while (application.fsm.state.sfunc != NULL)
    application.fsm.state.sfunc(&application.fsm.state, application.fsm.data);
```

7. Plugin finalization function is called.
This function allows the plugin to destroy its own resources.

8. Application resources are released by the application.

## User plugin

The user plugin interface is defined in `plugin.*.h`.
A plugin must implement 4 functions:

1. `help()`: used to print usage help when `--help` option is provided to the application.

2. `configure()`: used to select and configure features provided by the application,
and also to parse command line arguments for the plugin.

3. `init()`: used to initialize the plugin's own resources, and also to prepare
the initial finite state machine state.

4. `final()`: used to destroy the plugin's own resources, and set the application exit code.

It is possible to link `station-app` with a user plugin,
so that the latter becomes a standalone application.

## Contents of `station-app --help`

```
Usage: station-app [OPTION...] PLUGIN_FILE [-- [plugin arguments...]]

 Output options:
  -@, --logo                 Display application logo
  -C, --cl-list=TYPE         Display list of OpenCL-compatible hardware
                             (platforms, devices)
  -v, --verbose              Display more information

 Alternative modes:
  -H, --plugin-help          Display plugin help

 Feature options:
  -c, --cl-context=PID[:DMASK]   Create OpenCL context
                             (PID: platform index, DMASK: device mask)
  -f, --file=PATH            Open binary file for reading
  -j, --threads=[±]THREADS   Create concurrent processing context
                             (+: wait on condition variable, -: busy-wait)
  -l, --library=PATH         Open shared library
  -n, --no-sdl               Don't initialize SDL subsystems

 Signal management (interruption events):
      --SIGINT               Catch <interruption request>
      --SIGQUIT              Catch <quit request>
      --SIGTERM              Catch <termination request>

 Signal management (process events):
      --SIGCHLD              Catch <child stopped or terminated>
      --SIGCONT              Catch <continue if stopped>
      --SIGTSTP              Catch <stop request>
      --SIGXCPU              Catch <CPU time limit exceeded>
      --SIGXFSZ              Catch <file size limit exceeded>

 Signal management (input/output events):
      --SIGPIPE              Catch <broken pipe>
      --SIGPOLL              Catch <pollable event>
      --SIGURG               Catch <urgent condition on socket>

 Signal management (timer events):
      --SIGALRM              Catch <timer signal from alarm>
      --SIGPROF              Catch <profiling timer expired>
      --SIGVTALRM            Catch <virtual alarm clock>

 Signal management (terminal events):
      --SIGHUP               Catch <terminal hangup>
      --SIGTTIN              Catch <terminal input for background process>
      --SIGTTOU              Catch <terminal output for background process>
      --SIGWINCH             Catch <terminal resized>

 Signal management (user-defined):
      --SIGRTMAX=-n          Catch <real-time signal MAX-n>
      --SIGRTMIN=+n          Catch <real-time signal MIN+n>
      --SIGUSR1              Catch <user-defined signal 1>
      --SIGUSR2              Catch <user-defined signal 2>

  -?, --usage                Display a short usage message
  -h, --help                 Display this help list
      --version              Display application version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

## How to build

The project is built using the Ninja build system.
A `build.ninja` file is generated using `configure` script.
To see the possible build configuration options, run `configure` without arguments.
To generate `build.ninja`, run `configure <options>`.
To build, run `ninja`.

The configuration script also generates `station.pc` (pkg-config file for `libstation`)
and `station-app.pc` (pkg-config file for standalone executable user plugins).

## Build dependencies

* gcc-compatible compiler (like clang)
* pkg-config
* ninja

## Dependencies

The code implies the availability of the POSIX interfaces.

The following dependencies are optional.
Each of them is required only if the corresponding feature
is enabled at the build configuration step.

* SDL2
* OpenCL

## Examples

A quick and dirty demo plugin can be found in the `demo` subdirectory.

List of projects using Station:

* [still-alive](https://github.com/ivanp7/still-alive) (built as a standalone executable plugin)

## Documentation

Doxygen documentation is available at `docs` subdirectory. To build it, run `make`.

## License

```
   Station is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Station is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with Station. If not, see <http://www.gnu.org/licenses/>.
```

