add_library(memory_manager)
set(HEADER_FILES inc/scroom/blockallocator.hh inc/scroom/memoryblobs.hh)
target_sources(
  memory_manager
  PRIVATE src/blob-compression.cc
          src/blob-compression.hh
          src/memoryblobs.cc
          src/swapbasedblockallocator.cc
          ${HEADER_FILES})
target_link_libraries(
  memory_manager
  PRIVATE project_options
          project_warnings
          threadpool
          ZLIB::ZLIB
          fmt
  PUBLIC util)
target_include_directories(memory_manager PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
                                                 $<INSTALL_INTERFACE:include>)

install(
  TARGETS memory_manager
  EXPORT memory_manager
  INCLUDES
  DESTINATION include)
install(
  EXPORT memory_manager
  DESTINATION lib/cmake
  NAMESPACE scroom)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)

if(ENABLE_BOOST_TEST)
  add_executable(memory_manager_tests)
  target_sources(
    memory_manager_tests
    PRIVATE test/blob-tests.cc
            test/compression-tests.cc
            test/main.cc
            test/pageprovider-tests.cc
            test/swap-block-allocator-tests.cc)
  target_link_libraries(
    memory_manager_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            memory_manager
            Boost::system)
  target_include_directories(memory_manager_tests PRIVATE src)

  add_test(NAME memory_manager_tests COMMAND memory_manager_tests)
endif()
