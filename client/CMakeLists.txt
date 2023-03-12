find_package(Threads)

set(SOURCES async/main.cc)

add_executable(async ${SOURCES})
target_link_libraries(async PRIVATE asyncproto)