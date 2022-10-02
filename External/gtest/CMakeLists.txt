if(ENABLE_GOOGLE_TEST)
  # See https://google.github.io/googletest/quickstart-cmake.html
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_SHALLOW TRUE
    GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE
  )
  set(BUILD_SHARED_LIBS ON)
  FetchContent_MakeAvailable(googletest)
endif()
