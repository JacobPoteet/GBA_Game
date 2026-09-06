# Host-side unit tests for the Butano-free half of the game.
#
# This does NOT build the GBA ROM - that is the top level Makefile's job. It compiles the pure
# logic in src/gp_board.cpp and src/gp_cursor.cpp with the host compiler so the puzzle rules can
# be tested in milliseconds without an emulator.
#
#   cmake -S tests -B build-host
#   cmake --build build-host
#   ctest --test-dir build-host --output-on-failure

cmake_minimum_required(VERSION 3.16)
project(gba_game_tests CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

set(GAME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(gp_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_tests_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_board_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_cursor_test.cpp
    ${GAME_ROOT}/src/gp_board.cpp
    ${GAME_ROOT}/src/gp_cursor.cpp)

target_include_directories(gp_tests PRIVATE
    ${GAME_ROOT}/include
    ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(gp_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(gp_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME gp_tests COMMAND gp_tests)
