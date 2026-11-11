cmake_minimum_required(VERSION 3.16)

project(checkers_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(checkers_core
    src/agent.cpp
    src/board.cpp
    src/mailbox_abi.cpp)

target_include_directories(checkers_core PUBLIC include)
target_compile_options(checkers_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(checkers_core_fixtures tests/fixture_runner.cpp)
target_link_libraries(checkers_core_fixtures PRIVATE checkers_core)
target_include_directories(checkers_core_fixtures PRIVATE tests)
target_compile_options(checkers_core_fixtures PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME checkers_core_fixtures COMMAND checkers_core_fixtures)
