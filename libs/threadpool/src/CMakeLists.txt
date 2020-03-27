add_library(threadpool async-deleter.cc function-additor.cc queue.cc queue.hh threadpoolimpl.cc)
target_link_libraries(
        threadpool PRIVATE project_options project_warnings PUBLIC Boost::thread)
