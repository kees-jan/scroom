# Copilot instructions

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
