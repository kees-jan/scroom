#ifndef ASSERTIONS_H
#define ASSERTIONS_H

#include <sys/cdefs.h>

namespace Scroom
{
  namespace Utils
  {

    namespace Detail
    {

      /** Gets called when an assertion failed */
      void assertionFailed(const char *type, const char *expr,
          const char *function, const char *filename, unsigned int line)
              __attribute__ ((noreturn));
    }

  }
}

#if defined(OS_cygwin) || defined(OS_mingw) || defined(OS_msvc6)
#define __STRING(x)     #x
#endif

#define require(expr)                                                       \
  ((expr) ? ((void) 0) : Scroom::Utils::Detail::assertionFailed             \
   ("precondition", __STRING(expr), __PRETTY_FUNCTION__, __FILE__, __LINE__))
#define ensure(expr)                                                        \
  ((expr) ? ((void) 0) : Scroom::Utils::Detail::assertionFailed             \
   ("postcondition", __STRING(expr), __PRETTY_FUNCTION__,  __FILE__, __LINE__))
#define verify(expr)                                                        \
  ((expr) ? ((void) 0) : Scroom::Utils::Detail::assertionFailed             \
   ("assertion", __STRING(expr), __PRETTY_FUNCTION__,__FILE__, __LINE__))
#define defect()                                                            \
  Scroom::Utils::Detail::assertionFailed("control flow assertion",          \
                                     "", __PRETTY_FUNCTION__,__FILE__, __LINE__)



#endif
