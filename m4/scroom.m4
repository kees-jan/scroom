# SCROOM_SEARCH_LIBS(FUNCTION, SEARCH-LIBS,
#                    [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                    [OTHER-LIBRARIES])
# --------------------------------------------------------
# Search for a library defining FUNC, if it's not already available.
# Do not update the LIBS variable.
AC_DEFUN([SCROOM_SEARCH_LIBS],
[AS_VAR_PUSHDEF([scroom_Libs], [scroom_m4_$1])dnl
AS_VAR_COPY([scroom_Libs], [LIBS])
AC_SEARCH_LIBS([$1], [$2], [$3], [$4], [$5])
AS_VAR_COPY([LIBS], [scroom_Libs])
AS_VAR_POPDEF([scroom_Libs])dnl
])


# SCROOM_COMPILE_STDCXX_0X
AC_DEFUN([SCROOM_COMPILE_STDCXX_0X], [
  AC_CACHE_CHECK(if g++ supports C++0x features without additional flags,
  scroom_cv_cxx_compile_cxx0x_native,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type&& cr = check_type();],,
  scroom_cv_cxx_compile_cxx0x_native=yes, scroom_cv_cxx_compile_cxx0x_native=no)
  AC_LANG_RESTORE
  ])

  AC_CACHE_CHECK(if g++ supports C++0x features with -std=c++0x,
  scroom_cv_cxx_compile_cxx0x_cxx,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=c++0x"
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type&& cr = check_type();],,
  scroom_cv_cxx_compile_cxx0x_cxx=yes, scroom_cv_cxx_compile_cxx0x_cxx=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])

  AC_CACHE_CHECK(if g++ supports C++0x features with -std=gnu++0x,
  scroom_cv_cxx_compile_cxx0x_gxx,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  ac_save_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS -std=gnu++0x"
  AC_TRY_COMPILE([
  template <typename T>
    struct check
    {
      static_assert(sizeof(int) <= sizeof(T), "not big enough");
    };

    typedef check<check<bool>> right_angle_brackets;

    int a;
    decltype(a) b;

    typedef check<int> check_type;
    check_type&& cr = check_type();],,
  scroom_cv_cxx_compile_cxx0x_gxx=yes, scroom_cv_cxx_compile_cxx0x_gxx=no)
  CXXFLAGS="$ac_save_CXXFLAGS"
  AC_LANG_RESTORE
  ])
])

# Enable c++0x
# SCROOM_ENABLE_STDCXX_0X
AC_DEFUN([SCROOM_ENABLE_STDCXX_0X], [
  AC_REQUIRE([SCROOM_COMPILE_STDCXX_0X])
  AC_REQUIRE([AC_PROG_CXXCPP])
  if test "$scroom_cv_cxx_compile_cxx0x_native" = yes ||
     test "$scroom_cv_cxx_compile_cxx0x_cxx" = yes ||
     test "$scroom_cv_cxx_compile_cxx0x_gxx" = yes
  then
    AC_DEFINE(HAVE_STDCXX_0X,,[Define if g++ supports C++0x features. ])
  else
    AC_MSG_ERROR([Compiler not C++0x compatible])
  fi
  if test "$scroom_cv_cxx_compile_cxx0x_cxx" = yes
  then
    CXXFLAGS="-std=c++0x $CXXFLAGS"
    CXXCPP="$CXXCPP -std=c++0x"
  elif test "$scroom_cv_cxx_compile_cxx0x_gxx" = yes
  then
    CXXFLAGS="-std=gnu++0x $CXXFLAGS"
    CXXCPP="$CXXCPP -std=gnu++0x"
  fi
])

# SCOOM_ADD_CXXFLAGS_IF_SUPPORTED(flags)
# --------------------------------------------------------
# Try if the C++ compiler supports the given flags. Add them to
# CXXFLAGS if so.
AC_DEFUN([SCROOM_ADD_CXXFLAGS_IF_SUPPORTED], [
  AC_LANG_PUSH([C++])
  CXXFLAGS_KEEP="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS_KEEP $1"
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[]])], [CXXFLAGS_KEEP="$CXXFLAGS"])
  CXXFLAGS="$CXXFLAGS_KEEP"
  AC_LANG_POP([C++])
])

# SCOOM_TEST_ISYSTEM
# --------------------------------------------------------
# Try if the C++ compiler supports -isystem. Set ISYSTEM_FLAG to
# either -I or -isystem, depending on the outcome of the test
AC_DEFUN([SCROOM_TEST_ISYSTEM], [
  AC_CACHE_CHECK(if g++ supports -isystem,
  scroom_cv_cxx_compile_isystem,
  [AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  CXXFLAGS_KEEP="$CXXFLAGS"
  CXXFLAGS="-isystem . $CXXFLAGS_KEEP"
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([[]])],scroom_cv_cxx_compile_isystem=yes, scroom_cv_cxx_compile_isystem=no)
  CXXFLAGS="$CXXFLAGS_KEEP"
  AC_LANG_RESTORE
  ])

  if test "$scroom_cv_cxx_compile_isystem" = yes
  then
    ISYSTEM_FLAG="-isystem"
  else
    ISYSTEM_FLAG="-I"
  fi
])
