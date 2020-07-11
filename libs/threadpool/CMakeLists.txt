add_library(threadpool src/async-deleter.cc src/function-additor.cc src/queue.cc src/queue.hh src/threadpoolimpl.cc)
target_link_libraries(
        threadpool PRIVATE project_options project_warnings PUBLIC Boost::thread)

install(TARGETS threadpool EXPORT threadpool INCLUDES DESTINATION include)
install(EXPORT threadpool DESTINATION lib/cmake NAMESPACE scroom)

if (ENABLE_BOOST_TEST)
    add_executable(threadpool_tests test/main.cc test/async-deleter-tests.cc test/function-additor-tests.cc test/helpers.cc test/helpers.hh test/semaphore-tests.cc test/threadpool-destruction-tests.cc test/threadpool-queue-tests.cc test/threadpool-queueimpl-tests.cc test/threadpool-tests.cc)
    target_link_libraries(
            threadpool_tests PRIVATE boosttesthelper project_options project_warnings threadpool)
    target_include_directories(threadpool_tests PRIVATE src)

    if (ENABLE_SLOW_TESTS)
        add_test(NAME threadpool_tests COMMAND test/threadpool_tests)
    endif ()
endif ()
