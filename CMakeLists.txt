cmake_minimum_required(VERSION 3.15)

project(Scroom LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
include(UseBackportedModules)
include(GetGitRevisionDescription)

include(cmake/ProjectVersion.cmake)

include(cmake/StandardProjectSettings.cmake)

set(PLUGIN_INSTALL_LOCATION_RELATIVE "lib/scroom")
set(PLUGIN_INSTALL_LOCATION_ABSOLUTE "${CMAKE_INSTALL_PREFIX}/${PLUGIN_INSTALL_LOCATION_RELATIVE}")

set(PKG_DIR_RELATIVE "share/scroom")
set(PKG_DIR_ABSOLUTE "${CMAKE_INSTALL_PREFIX}/${PKG_DIR_RELATIVE}")

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
target_include_directories(project_options INTERFACE inc ${PROJECT_BINARY_DIR})
target_compile_definitions(project_options INTERFACE HAVE_CONFIG_H HAVE_VERSION_H)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" ON)
option(ENABLE_BOOST_TEST "Enable Boost Test builds" ON)
option(ENABLE_GOOGLE_TEST "Enable Google Test builds" ON)
option(ENABLE_SLOW_TESTS "Enable tests that take a lot of time" ON)
option(MULTITHREADING "Use as many threads as needed" ON)
option(DEBUG_TILES "Visualize internally used tiles" OFF)
option(XML_TEST_OUTPUT "Have all Boost unittests report in xml format" OFF)

# Very basic PCH example
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if (ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which
  # is a good idea if any #define's change
  # 
  # 
  target_precompile_headers(project_options INTERFACE <vector> <string> <map> <utility>)
endif()

# Set up some extra Conan dependencies based on our needs
# before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

if(ENABLE_GOOGLE_TEST)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES}
          gtest/1.10.0)
endif()

if(CPP_STARTER_USE_IMGUI)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES}
                           imgui-sfml/2.1@bincrafters/stable)

  # set(CONAN_EXTRA_OPTIONS ${CONAN_EXTRA_OPTIONS} sfml:shared=False
  # sfml:graphics=True sfml:audio=False sfml:window=True
  # libalsa:disable_python=True)
endif()

if(CPP_STARTER_USE_SDL)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES}
                           sdl2/2.0.10@bincrafters/stable)
  # set(CONAN_EXTRA_OPTIONS ${CONAN_EXTRA_OPTIONS} sdl2:wayland=True)
endif()


include(cmake/Conan.cmake)
run_conan()

if(ENABLE_BOOST_TEST OR ENABLE_GOOGLE_TEST)
  enable_testing()
endif()

if(ENABLE_BOOST_TEST)
  message("Building Boost Tests.")
endif()

if(ENABLE_GOOGLE_TEST)
  message("Building Google Tests.")
endif()

if(ENABLE_FUZZING)
  message(
    "Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html"
  )
  # add_subdirectory(fuzz_test)
endif()

configure_file(cmake-config.h.in config.h)

add_subdirectory(External)
add_subdirectory(inc)
add_subdirectory(libs)
add_subdirectory(plugins)
add_subdirectory(gui)

set(CPACK_PACKAGE_VERSION "${GIT_PROJECT_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)
