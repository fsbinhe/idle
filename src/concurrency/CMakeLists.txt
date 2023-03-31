add_library(libconcurrency executor.cc blocking_queue.cc)

target_link_libraries(libconcurrency pthread)