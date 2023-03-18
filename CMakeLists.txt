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

# Enable proto
add_subdirectory(proto)
add_subdirectory(client)

# find_package(LLVM CONFIG REQUIRED)
find_package(proxygen CONFIG REQUIRED)

find_package(zstd CONFIG REQUIRED)


# list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
# include(HandleLLVMOptions)
# add_definitions(${LLVM_DEFINITIONS})

# target_include_directories(main PRIVATE ${LLVM_INCLUDE_DIRS})

# # Find the libraries that correspond to the LLVM components that we wish to use
# llvm_map_components_to_libnames(llvm_libs Support Core IRReader ...)

# # Link against LLVM libraries
# target_link_libraries(main PRIVATE ${llvm_libs})

add_executable(idle src/asyncd.cc)
target_link_libraries(idle PRIVATE glog::glog)
target_link_libraries(idle PRIVATE gflags::gflags)
target_link_libraries(idle PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
target_link_libraries(idle PRIVATE asyncproto)
target_link_libraries(idle PRIVATE fmt::fmt)
target_link_libraries(idle PRIVATE leveldb::leveldb)
target_link_libraries(idle PRIVATE Folly::folly Folly::folly_deps Folly::follybenchmark Folly::folly_test_util)
target_link_libraries(idle PRIVATE proxygen::proxygen proxygen::proxygencurl proxygen::proxygenhttpserver)
target_link_libraries(idle PRIVATE $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>)
