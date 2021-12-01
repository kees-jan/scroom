#include <scroom/assertions.hh>

#ifdef _WIN32
#  include <boost/stacktrace.hpp>

#  include <scroom/unused.hh>
#else
#  include <execinfo.h>
#endif

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include <spdlog/spdlog.h>

namespace Scroom::Utils::Detail
{

  std::string stackTrace(size_t cutoff)
  {
    std::stringstream os;

#ifdef _WIN32
    // Stack traces generated by this are not very informative, they just show the DLLs.
    // Showing more than then DDLs would probably require an additional dependency
    os << boost::stacktrace::stacktrace();
    // Silence the compiler warning for unused variable
    UNUSED(cutoff);
#else
    os << "stack trace (innermost first):" << std::endl;
    static const size_t maxTraces = 100;
    void*               array[maxTraces];
    size_t              nTraces = backtrace(array, maxTraces);
    char**              strings = backtrace_symbols(array, static_cast<int>(nTraces));
    for(size_t i = cutoff + 1; i < nTraces; i++)
    {
      os << '#' << i << "  " << strings[i] << std::endl;
    }
    free(strings);
#endif
    return os.str();
  }

  void abort() __attribute__((noreturn));

  void assertionFailed(const std::string_view type,
                       const std::string_view expr,
                       const std::string_view function,
                       const std::string_view filename,
                       unsigned int           line)
  {
    spdlog::critical("PROGRAM DEFECTIVE: {}:{}: {} {} violated in {}", filename, line, type, expr, function);
    spdlog::critical("Stack trace: {}", stackTrace(1));

    abort();
  }

  class ErrorSignalHandler
  {
    friend void abort();

    static volatile sig_atomic_t isHandlerActive; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    static void                  handler(int /*sig*/);

  public:
    ErrorSignalHandler() noexcept;
  };

  volatile sig_atomic_t ErrorSignalHandler::isHandlerActive = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

  void ErrorSignalHandler::handler(int sig)
  {
    if(!isHandlerActive)
    {
      isHandlerActive = 1;
      std::cerr << __FILE__ << ":" << __LINE__ << ": Entering signal handler" << std::endl;
#ifdef _WIN32
      std::string_view sigName;
      switch(sig)
      {
      case SIGFPE:
        sigName = "SIGFPE";
        break;
      case SIGILL:
        sigName = "SIGILL";
        break;
      case SIGABRT:
        sigName = "SIGABRT";
        break;
      case SIGSEGV:
        sigName = "SIGSEGV";
        break;
      }
#else
      std::string_view sigName = strsignal(sig); // NOLINT(concurrency-mt-unsafe)
#endif
      std::cerr << "PROGRAM DEFECTIVE (TERMINATED BY SIGNAL): " << sigName << std::endl;
      std::cerr << stackTrace(1);
      std::cerr << std::endl;
      std::cerr << __FILE__ << ":" << __LINE__ << ": Leaving signal handler" << std::endl;
      std::cerr << std::flush;
    }

    signal(sig, SIG_DFL);
    raise(sig);
  }

  ErrorSignalHandler::ErrorSignalHandler() noexcept
  {
    signal(SIGFPE, handler);
    signal(SIGILL, handler);
    signal(SIGABRT, handler);
    signal(SIGSEGV, handler);
#ifndef _WIN32
    signal(SIGBUS, handler);
    signal(SIGSYS, handler);
#endif
  }

  static ErrorSignalHandler errorSignalHandler;

  void abort()
  {
    signal(SIGABRT, SIG_DFL);
    ::abort();
    signal(SIGABRT, errorSignalHandler.handler);
  }

} // namespace Scroom::Utils::Detail
