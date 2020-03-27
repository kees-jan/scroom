add_executable(threadpool_tests main.cc async-deleter-tests.cc function-additor-tests.cc helpers.cc helpers.hh semaphore-tests.cc threadpool-destruction-tests.cc threadpool-queue-tests.cc threadpool-queueimpl-tests.cc threadpool-tests.cc)
target_link_libraries(
        threadpool_tests PRIVATE boosttesthelper project_options project_warnings threadpool)
target_include_directories(threadpool_tests PRIVATE ../src)

if(ENABLE_SLOW_TESTS)
    add_test(NAME threadpool_tests COMMAND threadpool_tests)
endif()