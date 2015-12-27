Coding guidelines
=================

Naming conventions
------------------

*Types*
  Use ``PascalCase`` for all type definitions: classes, structs,
  namespaces, enums, and type aliases.

  .. code-block:: cpp

    class ThreadPool { ... };
    struct BitmapMetaData { ... };
    namespace Scroom::Utils { ... }
    enum State { Idle, Waiting, Working, Finished };
    using Ptr     = std::shared_ptr<ThreadPool>;
    using WeakPtr = std::weak_ptr<ThreadPool>;

  The conventional aliases ``Ptr`` and ``WeakPtr`` are defined in
  nearly every class.

  The exception is type aliases that follow the STL naming convention,
  such as ``value_type``.

*Functions and methods*
  Use ``camelCase``.

  .. code-block:: cpp

    void setProgress(State s);
    static Ptr instance();

*Variables*
  Use ``camelCase`` for both member variables and local variables.

  New code uses an ``m_`` prefix for member variables in classes (but
  not structs), and an ``s_`` prefix for static member variables. Old code
  uses no prefix.

  .. code-block:: cpp

    // New convention (class member variables)
    int m_blockCount;
    std::shared_ptr<spdlog::logger> m_logger;

    // New convention (static variables)
    static int s_instanceCount;

    // Old convention (still found throughout the codebase)
    int blockCount;
    std::shared_ptr<spdlog::logger> logger;

*Function arguments*
  Use ``camelCase``. When an argument would alias a member variable
  of the same name, add a trailing underscore to the argument name
  to avoid the clash.

  .. code-block:: cpp

    Segment(value_type start_, value_type size_)
      : start(start_)
      , size(size_)
    {}

*Enum values and global constants*
  Use ``UPPER_CASE_WITH_UNDERSCORES``.

  .. code-block:: cpp

    enum { PRIO_HIGHEST = 100, PRIO_HIGH, PRIO_NORMAL, PRIO_LOW };

Class layout
------------

Members are ordered as follows within a class:

1. ``public:`` or ``private:`` — type definitions
2. ``private:`` — member variables
3. ``public:`` or ``private:`` — member functions

Each section starts with an access specifier, even if it is redundant.

.. code-block:: cpp

  class LoggerContainer
  {
  public:
    using Ptr = std::shared_ptr<LoggerContainer>;

  private:
    mutable std::mutex              mut;
    std::shared_ptr<spdlog::logger> logger;

  public:
    static Ptr instance();
    LoggerContainer();
    std::shared_ptr<spdlog::logger> get() const;
    void                            set(std::shared_ptr<spdlog::logger> logger_);
  };

File names
----------

File names use ``kebab-case`` for names composed of multiple distinct
words (e.g., ``gtk-helpers.cc``, ``tiled-bitmap.hh``).
Compound words that are conventionally written as one word are not
split (e.g., ``threadpool.hh``, ``progressinterfacehelpers.hh``).

Formatting
----------

C++ formatting is done by ``clang-format-22``.
Formatting for ``CMakeLists.txt`` is done using ``cmake-format``
(``pip install cmake-format``).
Formatting is checked during the CI build.

Be sure to format your code before committing.
Commits that only change formatting are not likely to be accepted.
Most IDEs show the commit message next to the code, and a message
that reads "Updated formatting" does not really help the developers
after you.

Hence, I recommend you configure your IDE to format on every save.
Or maybe use a ``pre-commit`` hook.
Scroom doesn't currently have one, though, so patches are very
welcome 😉

Clang-tidy
----------

We use ``clang-tidy-22`` to enforce a consistent style and find
common bugs.
This, too, is checked during the CI build.

``clang-tidy`` currently reports a fair amount of warnings.
The idea is to reduce this number over time.

Note that ``clang-tidy`` is a relatively new development within
Scroom.
If a warning turns out to not make sense, turning it off
project-wide is certainly an option.
