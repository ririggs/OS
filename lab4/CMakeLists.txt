cmake_minimum_required(VERSION 3.16)
project(lab4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(_lab4_out ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${_lab4_out})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${_lab4_out})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${_lab4_out})

add_library(ipc_utils STATIC ipc_utils.cpp)
target_include_directories(ipc_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(lab4_receiver receiver.cpp)
target_link_libraries(lab4_receiver PRIVATE ipc_utils kernel32)

add_executable(lab4_sender sender.cpp)
target_link_libraries(lab4_sender PRIVATE ipc_utils kernel32)

add_dependencies(lab4_receiver lab4_sender)

enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL      https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(lab4_tests tests/test_ipc_utils.cpp)
target_link_libraries(lab4_tests PRIVATE ipc_utils GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab4_tests)
