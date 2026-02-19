FetchContent_Declare(
  libfmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_SHALLOW TRUE
  GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f # release v12.1.0
)
set(FMT_INSTALL ON)
set(BUILD_SHARED_LIBS ON)
FetchContent_MakeAvailable(libfmt)
