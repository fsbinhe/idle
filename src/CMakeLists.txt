
set(SOURCES internal/application.cc)

add_executable(async ${SOURCES})
target_link_libraries(async PRIVATE asyncproto raftproto)