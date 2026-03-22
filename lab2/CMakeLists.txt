cmake_minimum_required(VERSION 3.16)
project(lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(array_utils STATIC array_utils.cpp)
target_include_directories(array_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lab2_app main.cpp)
target_link_libraries(lab2_app PRIVATE array_utils)

target_link_libraries(lab2_app PRIVATE kernel32)

enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL      https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(lab2_tests tests/test_array_utils.cpp)
target_link_libraries(lab2_tests PRIVATE array_utils GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab2_tests)
