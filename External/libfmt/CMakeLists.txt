FetchContent_Declare(
  libfmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_SHALLOW TRUE
  GIT_TAG b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9 # release v8.1.1
)
set(FMT_INSTALL ON)
set(BUILD_SHARED_LIBS ON)
FetchContent_MakeAvailable(libfmt)
