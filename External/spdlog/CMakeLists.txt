FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_SHALLOW TRUE
  GIT_TAG 76fb40d95455f249bd70824ecfcae7a8f0930fa3 # release v1.10.0
)
set(SPDLOG_INSTALL ON)
set(SPDLOG_BUILD_SHARED ON)
set(SPDLOG_FMT_EXTERNAL ON)
FetchContent_MakeAvailable(spdlog)
