cmake_minimum_required(VERSION 3.16)

project(olj_xyz_engine LANGUAGES CXX)

add_library(olj_xyz_engine INTERFACE)

target_include_directories(
    olj_xyz_engine
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(olj_xyz_engine INTERFACE cxx_std_17)

add_executable(xyz_engine_example examples/main.cpp)
target_link_libraries(xyz_engine_example PRIVATE olj_xyz_engine)

option(OLJ_XYZ_ENGINE_BUILD_TESTS "Build xyz_engine tests" ON)

if(OLJ_XYZ_ENGINE_BUILD_TESTS)
    enable_testing()

    add_executable(xyz_engine_test tests/test_xyz_engine.cpp)
    target_link_libraries(xyz_engine_test PRIVATE olj_xyz_engine)

    add_test(NAME xyz_engine_test COMMAND xyz_engine_test)
endif()
