enable_testing()

find_package(GTest CONFIG REQUIRED)


add_library(hash hash.cc)

add_executable(hash_test hash_test.cc)
target_link_libraries(hash_test hash)
target_link_libraries(hash_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(hash_test)