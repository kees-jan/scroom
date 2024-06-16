# Copilot instructions

## Coding guidelines

The full coding guidelines are in `doc/coding-guidelines.rst`. Read that file
before working on C++ code in this repository.

## Formatting

After every code change, run the formatting scripts from the **root of the
repository** (not from a build directory, and without arguments — they find
files themselves):

```sh
build_scripts/clang-format   # formats all .cc/.hh files
build_scripts/cmake-format   # formats all CMakeLists.txt/.cmake files
```

Run `cmake-format` only when you have changed a `CMakeLists.txt` or `.cmake`
file. Do not make commits that consist solely of formatting changes; format as
you go.

## C++ style

- Do not specify template parameters that the compiler can deduce. For example,
  prefer `std::lock_guard const lock(mut)` over
  `std::lock_guard<std::mutex> const lock(mut)`.

- Name variables after the value they hold, not their type. For example, prefer
  `loggerContainer` over `container`. If two variables would otherwise get the
  same name, a type-related prefix or suffix is acceptable (e.g. `loggerContainer`
  to distinguish it from `logger`), but a name that is only a type description
  (e.g. `container`) is not.

## Testing

- Do not use singletons in tests if it can be avoided. Instead, construct the
  object under test locally and inject it into the code being tested. This keeps
  tests self-contained and avoids shared state between tests.

  For example, rather than calling `LoggerContainer::instance()` inside a test
  fixture, create a `LoggerContainer` directly and pass it to the `Logger`
  constructor:

  ```cpp
  // Prefer this:
  auto loggerContainer = std::make_shared<LoggerContainer>();
  Logger logger(loggerContainer);

  // Over this:
  Logger logger; // implicitly uses LoggerContainer::instance()
  ```
