cmake_minimum_required(VERSION 3.15)

project(Scroom LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")

include(UseBackportedModules)
include(GetGitRevisionDescription)

include(cmake/ProjectVersion.cmake)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

set(PLUGIN_INSTALL_LOCATION_RELATIVE "lib/scroom")
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
  # If a parent_scope exists, set the PLUGIN_INSTALL_LOCATION_RELATIVE accessible by the parent, so any plugins outside
  # of the base scroom directory
  set(PLUGIN_INSTALL_LOCATION_RELATIVE
      ${PLUGIN_INSTALL_LOCATION_RELATIVE}
      PARENT_SCOPE)
endif()
set(PLUGIN_INSTALL_LOCATION_ABSOLUTE "${CMAKE_INSTALL_PREFIX}/${PLUGIN_INSTALL_LOCATION_RELATIVE}")

set(PKG_DIR_RELATIVE "share/scroom")
set(PKG_DIR_ABSOLUTE "${CMAKE_INSTALL_PREFIX}/${PKG_DIR_RELATIVE}")

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)
target_include_directories(project_options INTERFACE inc ${PROJECT_BINARY_DIR})
target_compile_definitions(project_options INTERFACE HAVE_CONFIG_H HAVE_VERSION_H)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    target_compile_options(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

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
if(ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which is a good idea if any #define's change
  #
  # consider breaking this out per project as necessary
  target_precompile_headers(
    project_options
    INTERFACE
    <vector>
    <string>
    <map>
    <utility>)
endif()

set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

if(ENABLE_GOOGLE_TEST)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} gtest/1.10.0)
endif()

if(CPP_STARTER_USE_IMGUI)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} imgui-sfml/2.1@bincrafters/stable)

  # set(CONAN_EXTRA_OPTIONS ${CONAN_EXTRA_OPTIONS} sfml:shared=False sfml:graphics=True sfml:audio=False
  # sfml:window=True libalsa:disable_python=True)
endif()

if(CPP_STARTER_USE_SDL)
  set(CONAN_EXTRA_REQUIRES ${CONAN_EXTRA_REQUIRES} sdl2/2.0.10@bincrafters/stable)
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
  message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
  # add_subdirectory(fuzz_test)
endif()

set(PLUGINS
    ""
    CACHE INTERNAL "List of all plugins")
function(install_plugin_dependencies plugin-name)
  if(SCROOMBINDIR)
    list(APPEND PLUGINS "${SCROOMBINDIR}bin/lib${plugin-name}.dll")
  else()
    list(APPEND PLUGINS "bin/lib${plugin-name}.dll")
  endif()
  set(PLUGINS
      "${PLUGINS}"
      CACHE INTERNAL "List of all plugins")
endfunction()

configure_file(cmake-config.h.in config.h)
add_subdirectory(External)
add_subdirectory(inc)
add_subdirectory(libs)
add_subdirectory(plugins)
add_subdirectory(gui)

if(WIN32)
  message("Going to gather dependencies on Windows")
  install(CODE "set(PLUGINS \"${PLUGINS}\")")
  if(SCROOMBINDIR)
    install(CODE "set(SCROOMBINDIR \"${SCROOMBINDIR}\")")
  endif()
  install(
    CODE [[
      message("Gathering dependencies for: ${PLUGINS}")
      file(GET_RUNTIME_DEPENDENCIES
        EXECUTABLES ${SCROOMBINDIR}bin/scroom.exe
        LIBRARIES ${PLUGINS}
        RESOLVED_DEPENDENCIES_VAR resolved
        UNRESOLVED_DEPENDENCIES_VAR unresolved
        PRE_EXCLUDE_REGEXES "^api-ms-win.*" "^ext-ms-.*"
        POST_EXCLUDE_REGEXES "[Ww][Ii][Nn][Dd][Oo][Ww][Ss].[Ss][Yy][Ss][Tt][Ee][Mm]32/"
        DIRECTORIES c:/msys64/mingw64/bin)
      message("Resolved: ${resolved}")
      list(LENGTH unresolved unresolved_count)
      if(${unresolved_count} GREATER "0")
        message(SEND_ERROR "There are unresolved dependencies: ${unresolved}")
      endif()
      file(INSTALL ${resolved}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
  ]])
else()
  message(NOTICE "Not on Windows. Not gathering dependencies")
endif()

set(CPACK_PACKAGE_VERSION "${GIT_PROJECT_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)
