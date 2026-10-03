cmake_minimum_required(VERSION 3.16)

project(olj_binary_tree LANGUAGES CXX)

add_library(olj_binary_tree INTERFACE)

target_include_directories(
    olj_binary_tree
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(olj_binary_tree INTERFACE cxx_std_17)

add_executable(binary_tree_example examples/main.cpp)
target_link_libraries(binary_tree_example PRIVATE olj_binary_tree)

option(OLJ_BINARY_TREE_BUILD_TESTS "Build binary_tree tests" ON)

if(OLJ_BINARY_TREE_BUILD_TESTS)
    enable_testing()

    add_executable(binary_tree_test tests/test_binary_tree.cpp)
    target_link_libraries(binary_tree_test PRIVATE olj_binary_tree)

    add_test(NAME binary_tree_test COMMAND binary_tree_test)
endif()
