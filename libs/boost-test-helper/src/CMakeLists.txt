add_library(boosttesthelper STATIC unittest-main.cc)
target_link_libraries(
        boosttesthelper PRIVATE project_options project_warnings PUBLIC Boost::unit_test_framework)
