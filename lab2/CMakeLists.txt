cmake_minimum_required(VERSION 3.16)
project(lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use static CRT (/MT) to avoid runtime DLL dependency issues on Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ─────────────────────────────────────────────
# Shared logic library
# ─────────────────────────────────────────────
add_library(array_utils STATIC array_utils.cpp)
target_include_directories(array_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ─────────────────────────────────────────────
# Main executable
# ─────────────────────────────────────────────
add_executable(lab2_app main.cpp)
target_link_libraries(lab2_app PRIVATE array_utils)

# Link Windows system libraries required by WinAPI thread functions
target_link_libraries(lab2_app PRIVATE kernel32)

# ─────────────────────────────────────────────
# Unit tests with Google Test
# ─────────────────────────────────────────────
enable_testing()

include(FetchContent)

FetchContent_Declare(
    googletest
    URL      https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Must match the CRT setting above so GTest links against the same runtime
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(lab2_tests tests/test_array_utils.cpp)
target_link_libraries(lab2_tests PRIVATE array_utils GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(lab2_tests)
