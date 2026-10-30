# Local development guide

## Maintaining instructions

When updating skills or AGENTS.md, state the desired behavior directly. Add a
prohibition only when it supplies necessary guidance beyond the positive
instruction—for example, when overriding an established practice or a recurring
agent tendency. Prefer a general decision rule over accumulating restrictions
from individual mistakes.

## Commits and merge requests

When committing changes, pushing to GitHub, or creating or updating merge
requests, apply [commits-and-merge-requests](.agents/skills/commits-and-merge-requests/SKILL.md).

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

Use Ninja's default build parallelism; omit explicit job limits when building.

When a sandboxed build fails because ccache cannot write to its usual location,
request elevated sandbox permissions and rerun the normal preset build. Leave
ccache enabled and its configuration unchanged; do not disable it or redirect
its cache to work around sandbox restrictions.

In-source builds are forbidden. Use a separate build directory when switching
compilers. The `clang22-debug` preset builds and tests with Clang 22 and its
default system standard library. Use its build tree for `clang-tidy-22`: Clang
and GCC handle diagnostics differently, and clang-tidy has worked best with a
Clang-generated compilation database. Keep this preset building and passing
tests as well as the GCC preset.

## Tests

Build before running CTest. Run tests normally unless the session is known to
be headless, such as a CI build; in that case, supply a virtual display as in
Linux CI. When display availability is unknown, let the first normal test run
establish whether display setup is needed.

All test suites may run concurrently. Use the test presets' parallel settings
without overriding them with an explicit job limit.

GTK-dependent tests require a display even though these are unit tests. Xvfb
also needs `xauth`. If a normal test run reports `cannot open display`, address
display availability before investigating assertions. When using Xvfb, check
its startup if that error persists.

If sandboxed tests cannot access an existing display, request elevated sandbox
permissions and retry the normal CTest preset before considering Xvfb.

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

Names must accurately communicate responsibility. Distinguish values from
objects that operate on those values. Flag misleading existing names when they
affect your work. If the responsibility is unclear, ask before introducing or
propagating names. Apply the naming guidance in the `code-changes` skill when
changing code.

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
