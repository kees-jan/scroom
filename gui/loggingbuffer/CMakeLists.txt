add_library(loggingbuffer)
set(HEADER_FILES inc/scroom/ringbuffer-sink.hh)
target_sources(loggingbuffer PRIVATE src/ringbuffer-sink.cc ${HEADER_FILES})
target_link_libraries(
  loggingbuffer
  PRIVATE project_options project_warnings
  PUBLIC Boost::headers spdlog
)
target_include_directories(
  loggingbuffer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>
)

install(
  TARGETS loggingbuffer
  EXPORT loggingbuffer
  INCLUDES
  DESTINATION include
)
install(
  EXPORT loggingbuffer
  DESTINATION lib/cmake
  NAMESPACE scroom
)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)

if(ENABLE_GOOGLE_TEST)
  add_executable(loggingbuffer_tests)
  target_sources(loggingbuffer_tests PRIVATE test/ringbuffer-sink-tests.cc)
  target_link_libraries(
    loggingbuffer_tests
    PRIVATE gmock_main
            project_options
            project_warnings
            loggingbuffer
            threadpool
  )

  add_test(NAME loggingbuffer_tests COMMAND loggingbuffer_tests)
endif()
