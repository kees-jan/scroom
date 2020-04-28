# Scroom - Generic viewer for 2D data
# Copyright (C) 2009-2020 Kees-Jan Dijkzeul
# 
# SPDX-License-Identifier: LGPL-2.1

AC_PREREQ([2.64])
AC_INIT([Scroom],
        m4_esyscmd([build-aux/git-version-gen | tr -d '\n']),
        [https://github.com/kees-jan/scroom/issues/new],
        [scroom],
        [https://github.com/kees-jan/scroom/wiki])
        
# Detect the target system
AC_CANONICAL_HOST
case "${host_os}" in
    linux*)
        build_linux=yes
        AC_MSG_NOTICE([detected linux environment])
        ;;
    cygwin*|mingw*|msys*)
        build_windows=yes
        AC_MSG_NOTICE([detected windows environment])
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported. Or maybe the check in configure.ac is just broken."])
        ;;
esac
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])

# Make sure the configure script finds the compiler    
AM_COND_IF([WINDOWS], 
    [export PATH=$PATH:/c/msys64/mingw64/bin]) 
    
# The configure.ac from Kees-Jan also contained the serial-tests flag for using the serial test harness
# That one seems to be deprecated though and it seems to work without it
AM_INIT_AUTOMAKE([-Wall -Werror foreign silent-rules subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR
LT_PREREQ([2.2])
# LT_INIT([dlopen])
LT_INIT([win32-dll])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([inc/scroom/presentationinterface.hh])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
dnl AC_PROG_CC
AC_PROG_LN_S
dnl AC_PROG_INSTALL
AC_CHECK_PROG([CMAKE], [cmake], [yes], [no])

AC_LANG([C++])
SCROOM_COMPILE_STDCXX_0X
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wall])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-W])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wextra])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wconversion])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wpointer-arith])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wwrite-strings])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wunused])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wredundant-decls])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wuninitialized])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Winit-self])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wcast-qual])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wswitch])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Weffc++])
dnl SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wold-style-cast])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wfloat-equal])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wnon-virtual-dtor])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Woverloaded-virtual])
SCROOM_ADD_CXXFLAGS_IF_SUPPORTED([-Wsign-promo])
SCROOM_TEST_ISYSTEM

# MinGW doesn't support the -z flag
AM_COND_IF([LINUX], 
    [LDFLAGS="$LDFLAGS -Wl,-z,defs"])

AM_COND_IF([WINDOWS], 
    [AC_DEFINE([OS_mingw], [1], [Let make know we're on Windows])]) 
    
# The .pc files are at a weird place so make sure pkg-config finds them
AM_COND_IF([WINDOWS], 
    [export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/mingw64/lib/pkgconfig])    

# Configurable features
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
AC_ARG_ENABLE([xml-test-output],
  [AS_HELP_STRING([--enable-xml-test-output], 
    [Generate test results in XML format @<:@default=no@:>@])],
  [want_xml_test_output=${enableval}],
  [want_xml_test_output=no])
  

# Doxygen
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([$PACKAGE_NAME], [doxyfile], [doc])
  
# Checks for packages.
PKG_CHECK_MODULES([UTILS], [gtk+-2.0 >= 2.0.0])
PKG_CHECK_MODULES([TILEDBITMAP], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([SCROOMLIB], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([EXAMPLEPLUGIN], [gtk+-2.0 >= 2.0.0])
PKG_CHECK_MODULES([TRANSPARENTOVERLAYPLUGIN], [gtk+-2.0 >= 2.0.0])
PKG_CHECK_MODULES([COLORMAPPLUGIN], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([TIFFPLUGIN], [gtk+-2.0 >= 2.0.0, cairo])
PKG_CHECK_MODULES([GUI],        [gtk+-2.0 >= 2.0.0, libglade-2.0, gthread-2.0, gmodule-2.0, cairo])

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

# Checking for zlib
have_zlib_headers=no
have_zlib=no
AC_CHECK_HEADERS([zlib.h], [have_zlib_headers=yes])
SCROOM_SEARCH_LIBS([zlibVersion], [z], [have_zlib=yes])
AC_SUBST([ZLIB], [$ac_cv_search_zlibVersion])

# Checking for boost system
have_boost_system_lib=no
SCROOM_SEARCH_LIBS([_ZN5boost6system16generic_categoryEv], [boost_system-mt boost_system], [have_boost_system_lib=yes])
if test "x${have_boost_system_lib}" = xno
then
  ac_cv_search__ZN5boost6system16generic_categoryEv=""
fi
AC_SUBST([BOOSTSYSTEMLIB], ["$ac_cv_search__ZN5boost6system16generic_categoryEv"])

# Checking for pthreads
have_pthreads_lib=no
SCROOM_SEARCH_LIBS([pthread_mutexattr_init], [pthread], [have_pthreads_lib=yes])
AC_SUBST([PTHREADLIB], ["$ac_cv_search_pthread_mutexattr_init"])

# Checking for boost thread
have_boost_thread_headers=no
AC_CHECK_HEADERS([boost/thread.hpp boost/thread/mutex.hpp], [have_boost_thread_headers=yes])
have_boost_thread_lib=no
SCROOM_SEARCH_LIBS([_ZN5boost6thread6detachEv], [boost_thread-mt boost_thread], [have_boost_thread_lib=yes])
AC_SUBST([BOOSTTHREADLIB], ["$ac_cv_search__ZN5boost6thread6detachEv $ac_cv_search_pthread_mutexattr_init"])

#checking for boost unittests
have_boost_test_headers=no
AC_CHECK_HEADERS([boost/test/unit_test.hpp], [have_boost_test_headers=yes])
have_boost_test_lib=no
SCROOM_SEARCH_LIBS([_ZN5boost9unit_test16results_reporter10set_streamERSo],
                   [boost_unit_test_framework-mt boost_unit_test_framework], [have_boost_test_lib=yes])
AC_SUBST([BOOSTTESTLIB], [$ac_cv_search__ZN5boost9unit_test16results_reporter10set_streamERSo])

# Checking for boost futures
have_new_boost_futures=no
AC_CHECK_HEADERS([boost/thread/future.hpp], [have_new_boost_futures=yes], [],
[[#ifdef HAVE_BOOST_THREAD_HPP
# include <boost/thread.hpp>
#endif
]])

#Checking for boost program-options
have_boost_program_options_lib=no
have_boost_program_options_headers=no
SCROOM_SEARCH_LIBS([_ZN5boost15program_options19options_descriptionC2Ejj],
                   [boost_program_options-mt boost_program_options], [have_boost_program_options_lib=yes])
if test "x${ac_cv_search__ZN5boost15program_options19options_descriptionC2Ejj}" = xno
then
  ac_cv_search__ZN5boost15program_options19options_descriptionC2Ejj=""
else
  AC_CHECK_HEADERS([boost/program_options.hpp], [have_boost_program_options_headers=yes])
fi
AC_SUBST([BOOSTPROGRAMOPTIONSLIB], [$ac_cv_search__ZN5boost15program_options19options_descriptionC2Ejj])

#Checking for boost operators
have_boost_operators=no
AC_CHECK_HEADERS([boost/operators.hpp], [have_boost_operators=yes])

#Checking for boost filesystem
have_boost_filesystem_lib=no
have_boost_filesystem_headers=no
boost_filesystem_ldflags=""
# Mangled symbol for ubuntu 12.04
SCROOM_SEARCH_LIBS([_ZNK5boost11filesystem34path11parent_pathEv],
                   [boost_filesystem-mt boost_filesystem], [have_boost_filesystem_lib=yes])
if test "x${have_boost_filesystem_lib}" = xyes
then
  boost_filesystem_ldflags=${ac_cv_search__ZNK5boost11filesystem34path11parent_pathEv}
else
  # Mangled symbol for ubuntu 14.04 onwards
  SCROOM_SEARCH_LIBS([_ZNK5boost10filesystem4path11parent_pathEv],
                     [boost_filesystem-mt boost_filesystem], [have_boost_filesystem_lib=yes])
  if test "x${have_boost_filesystem_lib}" = xyes
  then
    boost_filesystem_ldflags=${ac_cv_search__ZNK5boost10filesystem4path11parent_pathEv}
  fi
fi
AC_CHECK_HEADERS([boost/filesystem.hpp], [have_boost_filesystem_headers=yes])
AC_SUBST([BOOSTFILESYSTEMLIB], [${boost_filesystem_ldflags}])

# Check if boost filesystem has canonical()
# Ubuntu 14.04 onwards should have it :-)
AC_CACHE_CHECK(if boost::filesystem exports the canonical() function,
  scroom_cv_boost_filesystem_canonical,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([
  #ifdef HAVE_BOOST_FILESYSTEM_HPP
  #include <boost/filesystem.hpp>
  #endif
  
  void f()
  {
  	boost::filesystem::path p;
  	(void)canonical(p);
  }],,
  scroom_cv_boost_filesystem_canonical=yes, scroom_cv_boost_filesystem_canonical=no)
  AC_LANG_RESTORE
  ])
have_boost_filesystem_canonical=${scroom_cv_boost_filesystem_canonical}

# Testing for google test
have_gtest_headers=no
AC_CHECK_HEADERS([gtest/gtest.h], [have_gtest_headers=yes])
gtest_location=/usr/src/gtest
have_gtest_sources=no
AC_CHECK_FILE([/usr/src/gtest/CMakeLists.txt], [have_gtest_sources=yes;gtest_location=/usr/src/gtest])
AC_CHECK_FILE([/usr/src/googletest/googletest/CMakeLists.txt], [have_gtest_sources=yes;gtest_location=/usr/src/googletest/googletest])

# Testing for google mock
have_gmock_headers=no
AC_CHECK_HEADERS([gmock/gmock.h], [have_gmock_headers=yes])
gmock_location=/usr/src/gmock
have_gmock_sources=no
AC_CHECK_FILE([/usr/src/gmock/CMakeLists.txt], [have_gmock_sources=yes;gmock_location=/usr/src/gmock])
AC_CHECK_FILE([/usr/src/googletest/googlemock/CMakeLists.txt], [have_gmock_sources=yes;gmock_location=/usr/src/googletest/googlemock])

# Some sanity checks
if test "x${have_new_boost_futures}" = xno
then
  AC_MSG_ERROR([boost/thread/future.hpp not found])
fi
if test "x${have_zlib_headers}" = xno
then
  AC_MSG_ERROR([zLib headers not found])
fi
if test "x${have_zlib}" = xno
then
  AC_MSG_ERROR([zLib library not found])
fi
if test "x${have_boost_thread_headers}" = xno
then
  AC_MSG_ERROR([Boost thread header files not found.])
fi
if test "x${have_boost_thread_lib}" = xno
then
  AC_MSG_ERROR([Boost thread library not found.])
fi
if test "x${have_boost_operators}" = xno
then
  AC_MSG_ERROR([Boost operators header file not found.])
fi
if test "x${have_pthreads_lib}" = xno
then
  AC_MSG_ERROR([Pthreads library not found.])
fi
if test "x${have_boost_filesystem_headers}" = xno
then
  AC_MSG_ERROR([Boost filesystem header files not found.])
fi
if test "x${have_boost_filesystem_lib}" = xno
then
  AC_MSG_ERROR([Boost filesystem library not found.])
fi

want_boost_tests=no
if test x$have_boost_test_headers = xyes -a x$have_boost_test_lib = xyes
then
  want_boost_tests=yes
fi
AM_CONDITIONAL([BOOST_UNITTESTS], [test x$want_boost_tests = xyes])

want_gtest=no
if test "x${CMAKE}" = xyes -a \
   "x${have_gtest_headers}" = xyes -a "x${have_gtest_sources}" = xyes -a \
   "x${have_gmock_headers}" = xyes -a "x${have_gmock_sources}" = xyes
then
  want_gtest=yes
fi
AM_CONDITIONAL([GOOGLE_UNITTESTS], [test x$want_gtest = xyes])
AC_SUBST([GTEST_LOCATION], [$gtest_location])
AC_SUBST([GMOCK_LOCATION], [$gmock_location])

# If no doxygen, then we can't process all code
if DX_TEST_FEATURE([doc])
then
    want_doxygen=yes
else
    want_doxygen=no
    want_doxygen_all_code=no
fi

SCROOM_ENABLE_STDCXX_0X

if test "x${want_doxygen_all_code}" = xyes
then
  DX_EXTRA_SRC=`find $srcdir -name src -type d | tr '\n' ' '`
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
if test "x${want_xml_test_output}" = xyes
then
  AC_DEFINE([XML_TEST_OUTPUT], 1, [Have all Boost unittests report in xml format])
fi
if test "x${have_boost_filesystem_canonical}" = xyes
then
  AC_DEFINE([HAVE_BOOST_FILESYSTEM_CANONICAL], 1, [Boost filesystem exports a canonical() function])
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
                 gui/ENVIRONMENT
                 doxyfile
])

# Misc defines
AC_SUBST([plugindir], [$libdir/$PACKAGE])
AC_SUBST([incscroomdir], [$includedir/scroom])
AC_SUBST([incscroomimpldir], [$incscroomdir/impl])
AC_SUBST([incscroomgmockdir], [$incscroomdir/gmock])

AC_OUTPUT


echo \
"-------------------------------------------------

 ${PACKAGE_NAME} version ${PACKAGE_VERSION}

 Prefix:   '${prefix}'.
 Compiler: '${CXX} ${CXXFLAGS} ${CPPFLAGS}'
           
 Package features:
   Doxygen:         ${want_doxygen}
     all code:      ${want_doxygen_all_code}
   Unittests:
     boost:         ${want_boost_tests}
     google:        ${want_gtest}
   Debug-tiles:     ${want_debug_tiles}
   MultiThreading:  ${want_multithreading}

 Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all            - build all binaries
     install        - install everything
     doxygen-doc    - generate documentation
                      (if enabled)
     check          - perform tests

--------------------------------------------------"
