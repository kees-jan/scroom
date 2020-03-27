find_package(Threads)

add_library(pthreads INTERFACE IMPORTED GLOBAL)
target_link_libraries(pthreads INTERFACE ${CMAKE_THREAD_LIBS_INIT})
