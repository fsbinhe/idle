find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

# Async Service
set(ASYNC_PROTO_FILES
  async/v1/async.proto)

add_library(asyncproto ${ASYNC_PROTO_FILES})

target_link_libraries(asyncproto
  PUBLIC 
    protobuf::libprotobuf
    gRPC::grpc
    gRPC::grpc++
)
target_include_directories(asyncproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET asyncproto LANGUAGE cpp)
protobuf_generate(TARGET asyncproto LANGUAGE grpc 
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

# Raft Service
set(RAFT_PROTO_FILES
  raft/v1/raft.proto
)

add_library(raftproto ${RAFT_PROTO_FILES})

target_link_libraries(raftproto
  PUBLIC 
    protobuf::libprotobuf
    gRPC::grpc
    gRPC::grpc++
)
target_include_directories(raftproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET raftproto LANGUAGE cpp)
protobuf_generate(TARGET raftproto LANGUAGE grpc 
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc 
  PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)