# Local development guide

## Project

Scroom is a cross-platform C++23 GTK 3 image viewer with dynamically loaded
plugins. It must run on all currently supported Ubuntu versions and on Windows.
Use the CMake build; see `.github/workflows/build_cmake.yml` for platform build
and test details.

On the developer's PC, dependencies are already installed in standard system
locations. Start with normal CMake discovery.

CMake FetchContent downloads pinned dependencies even with system development
packages installed; the initial configure needs network access to GitHub.

## Configure and compile

The CMake modules live in the `cmake-modules` submodule, which must be initialized.
Version generation requires full Git history and tags.

CMake presets are available in `CMakePresets.json`; `gcc-debug` is the default
choice. The Debug build directory contains `bin/`,
`lib/`, and `compile_commands.json`.

In-source builds are forbidden. Use a separate build directory when switching
compilers. The `clang22-libcxx-gcc13-debug` preset uses Clang 22 with **libstdc++**,
despite its name, and pins `/usr/lib/gcc/x86_64-linux-gnu/13`; use it only if that
toolchain exists. The GCC preset is the simpler default.

## Tests

Build before running CTest. For headless sessions, supply a virtual display as in Linux CI.

GTK-dependent tests require a display even though these are unit tests. Xvfb
also needs `xauth`. A `cannot open display` failure is an environment problem;
check the display/Xvfb startup before investigating assertions.

`ENABLE_GOOGLE_TEST` and `ENABLE_SLOW_TESTS` default to `ON`. There are currently
eight CTest entries: `util_tests`, `threadpool_tests`, `memory_manager_tests`,
`scroom_lib_tests`, `tiledbitmap_tests`, `sppipette_tests`,
`transparentoverlay_tests`, and `ruler_tests`.

Disabling `ENABLE_SLOW_TESTS` removes the threadpool suite from CTest registration;
it does not merely filter individual slow cases. Reconfigure to change options.

## Run the application without installing

Source the generated `ENVIRONMENT` file in the build directory before launching
the application.

The generated file sets `SCROOM_DEV_MODE` and `SCROOM_PLUGIN_DIRS`. Development
mode reads `gui/scroom.glade` directly from the source tree and loads plugins
from the build's `lib/` directory on Linux. Launching the executable without
this environment instead searches the configured installation locations, which
can hide new plugins or fail to find UI resources. The generated plugin path
contains a deliberate bogus entry; the following build directory is the useful
entry. `SCROOM_PLUGIN_DIRS` is colon-separated on Linux, semicolon-separated on
Windows. Development mode checks whether `SCROOM_DEV_MODE` exists; disabling it
requires removing the variable from the environment.

A useful manual smoke test is to open
a small PPM or TIFF, confirm the image and loaded-plugin log, then exercise zoom,
pan, and the UI feature being changed. For headless startup checks, use
a virtual display with the development environment active; the GUI keeps running until
closed, so a timeout alone does not prove that an image loaded successfully.

Installation resource and plugin paths are compiled in, so choose the installation
prefix before building. Clear the development environment variables when testing
an installation.

## Code map and checks

- `gui/src/`: startup, plugin loading, GTK callbacks, and views;
  `gui/scroom.glade`: UI; `gui/ruler/`: ruler widget and tests.
- `libs/`: utility/context types, threadpool, memory manager, tiled bitmap,
  shared viewer logic, and plugin interfaces. Public headers live under the
  corresponding library's `inc/scroom/` directory.
- `plugins/`: image loaders (TIFF/PNM), overlays, measurement, pipette, metadata,
  and colormap tools. Tests generally live alongside their component.
- `External/`: system dependency discovery and pinned FetchContent declarations.

For code changes, build and run the relevant suite, then the full suite when
shared libraries or interfaces change. Match existing formatting. CI uses
`clang-format-22` via `build_scripts/clang-format` and `cmake-format` via
`build_scripts/cmake-format`; both scripts modify files and require GNU parallel.
Prefer formatting only touched files to avoid unrelated changes (the C++ script
recursively scans the current directory). Do not commit build outputs or edit
the `cmake-modules` submodule for ordinary application changes.
