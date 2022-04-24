if(ENABLE_GOOGLE_TEST)
  # See https://google.github.io/googletest/quickstart-cmake.html
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_SHALLOW TRUE
    GIT_TAG e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS ON)
  FetchContent_MakeAvailable(googletest)
endif()
