cmake_minimum_required(VERSION 3.15)

project(idle)

# Enable generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable use vcpkg integration
set(CMAKE_TOOLCHAIN_FILE "/home/hebin/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "vcpkg toolchain file")

# Enable C++ 17
set(CMAKE_CXX_STANDARD 17)

find_package(gflags REQUIRED)
find_package(glog 0.6.0 REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Enable proto
add_subdirectory(proto)
add_subdirectory(client)

add_executable(idle src/asyncd.cc)

target_link_libraries(idle PRIVATE glog::glog)
target_link_libraries(idle PRIVATE gflags::gflags)
target_link_libraries(idle PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)
target_link_libraries(idle PRIVATE asyncproto)