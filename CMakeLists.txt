cmake_minimum_required(VERSION 3.15)

project(idle)

# Enable generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable use vcpkg integration
set(CMAKE_TOOLCHAIN_FILE "/home/hebin/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "vcpkg toolchain file")

# Enable C++ 17
set(CMAKE_CXX_STANDARD 17)

find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(leveldb CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(proxygen CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)

# Enable proto
add_subdirectory(proto)
add_subdirectory(client)
add_subdirectory(src)


add_executable(asyncd src/asyncd.cc)
target_link_libraries(asyncd PRIVATE hash)
target_link_libraries(asyncd PRIVATE glog::glog)
target_link_libraries(asyncd PRIVATE gflags::gflags)
target_link_libraries(asyncd PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
target_link_libraries(asyncd PRIVATE asyncproto)
target_link_libraries(asyncd PRIVATE fmt::fmt)
target_link_libraries(asyncd PRIVATE leveldb::leveldb)
target_link_libraries(asyncd PRIVATE Folly::folly Folly::folly_deps Folly::follybenchmark Folly::folly_test_util)
target_link_libraries(asyncd PRIVATE proxygen::proxygen proxygen::proxygencurl proxygen::proxygenhttpserver)
target_link_libraries(asyncd PRIVATE $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>)



add_executable(raftd src/raftd.cc)
target_link_libraries(raftd PRIVATE libraft)
target_link_libraries(raftd PRIVATE hash)
target_link_libraries(raftd PRIVATE glog::glog)
target_link_libraries(raftd PRIVATE gflags::gflags)
target_link_libraries(raftd PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
target_link_libraries(raftd PRIVATE asyncproto)
target_link_libraries(raftd PRIVATE fmt::fmt)
target_link_libraries(raftd PRIVATE leveldb::leveldb)
target_link_libraries(raftd PRIVATE Folly::folly Folly::folly_deps Folly::follybenchmark Folly::folly_test_util)
target_link_libraries(raftd PRIVATE proxygen::proxygen proxygen::proxygencurl proxygen::proxygenhttpserver)
target_link_libraries(raftd PRIVATE $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>)