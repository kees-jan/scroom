add_library(threadpool)
set(HEADER_FILES
    inc/scroom/async-deleter.hh
    inc/scroom/function-additor.hh
    inc/scroom/semaphore.hh
    inc/scroom/threadpool.hh
)
target_sources(
  threadpool
  PRIVATE src/async-deleter.cc
          src/function-additor.cc
          src/queue.cc
          src/queue.hh
          src/threadpool.cc
          ${HEADER_FILES}
)
target_link_libraries(
  threadpool
  PRIVATE project_options
          project_warnings
          spdlog
          util
  PUBLIC Boost::thread
)
target_include_directories(
  threadpool PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>
)

install(
  TARGETS threadpool
  EXPORT threadpool
  INCLUDES
  DESTINATION include
)
install(
  EXPORT threadpool
  DESTINATION lib/cmake
  NAMESPACE scroom
)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)

if(ENABLE_GOOGLE_TEST)
  add_executable(threadpool_tests)
  target_sources(
    threadpool_tests
    PRIVATE test/async-deleter-tests.cc
            test/function-additor-tests.cc
            test/helpers.cc
            test/helpers.hh
            test/semaphore-tests.cc
            test/threadpool-destruction-tests.cc
            test/threadpool-queue-tests.cc
            test/threadpool-queueimpl-tests.cc
            test/threadpool-tests.cc
  )
  target_link_libraries(
    threadpool_tests
    PRIVATE gmock_main
            project_options
            project_warnings
            threadpool
  )
  target_include_directories(threadpool_tests PRIVATE src)

  if(ENABLE_SLOW_TESTS)
    add_test(NAME threadpool_tests COMMAND threadpool_tests)
  endif()
endif()
