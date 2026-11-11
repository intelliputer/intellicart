cmake_minimum_required(VERSION 3.16)

project(checkers_board_geometry LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(checkers_board_geometry src/geometry.cpp)
target_include_directories(checkers_board_geometry PUBLIC include)
target_compile_options(checkers_board_geometry PRIVATE -Wall -Wextra -Wpedantic)

add_executable(board_geometry_fixtures tests/fixture_runner.cpp)
target_link_libraries(board_geometry_fixtures PRIVATE checkers_board_geometry)
target_compile_options(board_geometry_fixtures PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME board_geometry_fixtures COMMAND board_geometry_fixtures)
