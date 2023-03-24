enable_testing()
find_package(GTest CONFIG REQUIRED)

add_library(libraft raft_node.cc raft_storage.cc)
target_link_libraries(libraft raftproto)
target_link_libraries(libraft gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
target_link_libraries(libraft glog::glog)
target_link_libraries(libraft libconcurrency)


add_executable(raft_storage_test raft_storage_test.cc)
target_link_libraries(raft_storage_test GTest::gtest_main)
target_link_libraries(raft_storage_test libraft)

include(GoogleTest)
gtest_discover_tests(raft_storage_test)