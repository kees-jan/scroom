FetchContent_Declare(
  libfmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_SHALLOW TRUE
  GIT_TAG f5e54359df4c26b6230fc61d38aa294581393084 # release v10.1.1
)
set(FMT_INSTALL ON)
set(BUILD_SHARED_LIBS ON)
FetchContent_MakeAvailable(libfmt)
