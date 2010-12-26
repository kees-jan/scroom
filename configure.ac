# Scroom - Generic viewer for 2D data
# Copyright (C) 2009-2010 Kees-Jan Dijkzeul
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License, version 2, as published by the Free Software Foundation.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

AC_PREREQ([2.64])
AC_INIT([Scroom],
        m4_esyscmd([build-aux/git-version-gen | tr -d '\n']),
        [http://fuzzy.homedns.org/trac/scroom/newticket],
        [scroom],
        [http://fuzzy.homedns.org/trac/scroom])
AM_INIT_AUTOMAKE([-Wall -Werror foreign silent-rules])
AM_SILENT_RULES([yes])
LT_PREREQ([2.2])
LT_INIT([dlopen])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([inc/scroom/presentationinterface.hh])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
dnl AC_PROG_CC
AC_PROG_LN_S
dnl AC_PROG_INSTALL

AC_LANG([C++])
SCROOM_COMPILE_STDCXX_0X
SCROOM_ENABLE_STDCXX_0X
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wall])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-W])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wextra])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wconversion])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wpointer-arith])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wwrite-strings])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wunused])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wno-multichar])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wredundant-decls])


# Configurable features
AC_ARG_ENABLE([mutracx-hacks],
  [AS_HELP_STRING([--enable-mutracx-hacks], 
    [Build for a Mutracx environment @<:@default=no@:>@])],
  [want_mutracx_hacks=${enableval}],
  [want_mutracx_hacks=unknown])
AC_ARG_ENABLE([doxygen-all-code],
  [AS_HELP_STRING([--enable-doxygen-all-code], 
    [Generate doxygen documentation for all code, instead of just the api @<:@default=no@:>@])],
  [want_doxygen_all_code=${enableval}],
  [want_doxygen_all_code=no])
AC_ARG_ENABLE([debug-tiles],
  [AS_HELP_STRING([--enable-debug-tiles], 
    [Display the (internally used) tiles (Debugging) @<:@default=no@:>@])],
  [want_debug_tiles=${enableval}],
  [want_debug_tiles=no])
AC_ARG_ENABLE([multithreading],
  [AS_HELP_STRING([--disable-multithreading], 
    [Use as few threads as possible (Debugging) @<:@default=no@:>@])],
  [want_multithreading=${enableval}],
  [want_multithreading=yes])
  

# Doxygen
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([$PACKAGE_NAME], [doxyfile], [doc])
  
# Checks for packages.

#PKG_CHECK_MODULES([HEADER, [gtk+-2.0 >= 2.0.0, libglade-2.0 glib-2.0 cairo])
PKG_CHECK_MODULES([TILEDBITMAP], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([EXAMPLEPLUGIN], [gtk+-2.0 >= 2.0.0])
PKG_CHECK_MODULES([COLORMAPPLUGIN], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([TIFFPLUGIN], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([GUI],        [gtk+-2.0 >= 2.0.0, libglade-2.0, gthread-2.0, cairo])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset pow select sqrt strerror])

# Mutracx specifics:
AC_MSG_CHECKING([for Mutracx development environment])
need_mutracx_hacks=no
if pkg-config "gtk+-2.0 >= 2.16.0"
then
  need_mutracx_hacks=no
  AC_MSG_RESULT([no])
else
  need_mutracx_hacks=yes
  AC_MSG_RESULT([yes])
fi

# Checking for boost thread
have_boost_thread_headers=no
AC_CHECK_HEADERS([boost/thread.hpp boost/thread/mutex.hpp], [have_boost_thread_headers=yes])
have_boost_thread_lib=no
# boost_thread-gcc42-mt is the name used at Mutracx...
SCROOM_SEARCH_LIBS([_ZN5boost6thread6detachEv], [boost_thread-mt boost_thread-gcc42-mt], [have_boost_thread_lib=yes])
AC_SUBST([BOOSTTHREADLIB], [$ac_cv_search__ZN5boost6thread6detachEv])

#checking for boost unittests
have_boost_test_headers=no
AC_CHECK_HEADERS([boost/test/included/unit_test.hpp boost/test/unit_test.hpp], [have_boost_test_headers=yes])
have_boost_test_lib=no
# SCROOM_SEARCH_LIBS([],
#                    [boost_unit_test_framework-mt libboost_unit_test_framework-gcc42-mt], [have_boost_test_lib=yes])

# Checking for boost futures
have_new_boost_futures=no
AC_CHECK_HEADERS([boost/thread/future.hpp], [have_new_boost_futures=yes], [],
[[#ifdef HAVE_BOOST_THREAD_HPP
# include <boost/thread.hpp>
#endif
]])
have_old_boost_futures=no
AC_CHECK_HEADERS([boost/future.hpp], [have_old_boost_futures=yes])

boost_futures_include_flags=""
if test "x${have_new_boost_futures}" = xyes
then
  AC_DEFINE([NEW_BOOST_FUTURES], 1, [Use futures from the Boost library release])
else
  if test "x${have_old_boost_futures}" = xno
  then
    boost_futures_include_flags="-I \$(top_srcdir)/compat/boost-futures/inc"
  fi
fi
AC_SUBST([BOOST_FUTURES_CPPFLAGS], [$boost_futures_include_flags])
AM_CONDITIONAL([NEW_BOOST_FUTURES], [test x$have_new_boost_futures = xyes])

# Some sanity checks
if test "x${have_boost_thread_headers}" = xno
then
  AC_MSG_ERROR([Boost thread header files not found.])
fi
if test "x${have_boost_thread_lib}" = xno
then
  AC_MSG_ERROR([Boost thread library not found.])
fi
want_boost_tests=$have_boost_test_headers
AM_CONDITIONAL([BOOST_UNITTESTS], [test x$want_boost_tests = xyes])
if test "x${need_mutracx_hacks}" = xyes
then
  # Print a warning if the user said "no" to mutracx hacks. If he said
  # "yes", or nothing at all, be quiet and enable anyway.
  if test "x${want_mutracx_hacks}" = xno
  then
    AC_MSG_WARN([Mutracx environment detected. Enabling mutracx-hacks])
  fi
  want_mutracx_hacks=yes
elif test "x${want_mutracx_hacks}" = xunknown
then
  want_mutracx_hacks=$need_mutracx_hacks
fi

# If no doxygen, then we can't process all code
if DX_TEST_FEATURE([doc])
then
    want_doxygen=yes
else
    want_doxygen=no
    want_doxygen_all_code=no
fi


# want_mutracx_hacks is now set to "yes" if they are either needed or
# requested.
if test "x${want_mutracx_hacks}" = xyes
then
  AC_DEFINE([MUTRACX_HACKS], 1, [Hacks for Mutracx environment enabled])
fi

if test "x${want_doxygen_all_code}" = xyes
then
  DX_EXTRA_SRC="\$(top_srcdir)/gui \$(top_srcdir)/libs \$(top_srcdir)/plugins"
else
  DX_EXTRA_SRC=
fi
AC_SUBST(DX_EXTRA_SRC)

if test "x${want_debug_tiles}" = xyes
then
  AC_DEFINE([DEBUG_TILES], 1, [Visualize internally used tiles])
fi
if test "x${want_multithreading}" = xyes
then
  AC_DEFINE([MULTITHREADING], 1, [Use as many threads as needed])
fi


# GetText
dnl GETTEXT_PACKAGE=scroom
dnl AC_SUBST(GETTEXT_PACKAGE)
dnl AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
dnl 
dnl dnl Add the languages which your application supports here.
dnl ALL_LINGUAS=""
dnl AM_GLIB_GNU_GETTEXT


AC_CONFIG_FILES([Makefile
                 compat/Makefile
                 compat/boost-futures/Makefile
                 compat/boost-futures/inc/Makefile
                 compat/boost-futures/test/Makefile
                 inc/Makefile
                 libs/Makefile
                 libs/threadpool/Makefile
                 libs/threadpool/src/Makefile
                 libs/threadpool/test/Makefile
                 libs/memory-manager/Makefile
                 libs/tiled-bitmap/Makefile
                 plugins/Makefile
                 plugins/colormap/Makefile
                 plugins/example/Makefile
                 plugins/tiff/Makefile
                 gui/Makefile
                 gui/ENVIRONMENT
                 gui/src/Makefile
                 doxyfile
])

# Misc defines
AC_SUBST([plugindir], [$libdir/$PACKAGE])

AC_OUTPUT


echo \
"-------------------------------------------------

 ${PACKAGE_NAME} version ${PACKAGE_VERSION}

 Prefix:   '${prefix}'.
 Compiler: '${CXX} ${CXXFLAGS} ${CPPFLAGS}'
           
 Package features:
   Mutracx hacks: ${want_mutracx_hacks}
   Doxygen:       ${want_doxygen}
     all code:    ${want_doxygen_all_code}
   Unittests:     ${want_boost_tests}

 Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all                - build all binaries
     install            - install everything
     doxygen-doc        - generate documentation
                          (if enabled)
     check              - perform tests

--------------------------------------------------"


