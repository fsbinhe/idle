find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

set(PROTO_FILES
  async/v1/async.proto)

add_library(asyncproto ${PROTO_FILES})
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

