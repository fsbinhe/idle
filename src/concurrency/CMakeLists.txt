add_library(libconcurrency executor.cc)

target_link_libraries(libconcurrency pthread)