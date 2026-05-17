cmake_minimum_required(VERSION 3.14)
project(employee_pipe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(employee_core STATIC
    types.h
    lock_manager.cpp
    file_manager.cpp
)
target_include_directories(employee_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    add_executable(server
        server.cpp
        common.h
    )
    target_link_libraries(server PRIVATE employee_core ws2_32)

    add_executable(client
        client.cpp
        common.h
    )
    target_link_libraries(client PRIVATE employee_core ws2_32)
endif()

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_lock_manager
    tests/test_lock_manager.cpp
)
target_link_libraries(test_lock_manager PRIVATE employee_core gtest_main)

add_executable(test_file_manager
    tests/test_file_manager.cpp
)
target_link_libraries(test_file_manager PRIVATE employee_core gtest_main)

include(GoogleTest)
gtest_discover_tests(test_lock_manager)
gtest_discover_tests(test_file_manager)
