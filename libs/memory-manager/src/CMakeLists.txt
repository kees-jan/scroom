add_library(memory_manager blob-compression.cc blob-compression.hh memoryblobs.cc swapbasedblockallocator.cc)
target_link_libraries(
        memory_manager PRIVATE project_options project_warnings util threadpool ZLIB::ZLIB)

install(TARGETS memory_manager EXPORT memory_manager INCLUDES DESTINATION include)
install(EXPORT memory_manager DESTINATION lib/cmake NAMESPACE scroom)