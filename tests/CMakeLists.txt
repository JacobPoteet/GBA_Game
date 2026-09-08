# Host-side unit tests for the Butano-free half of the game.
#
# This does NOT build the GBA ROM - that is the top level Makefile's job. It compiles the pure
# simulation in src/ with the host compiler so the rules can be tested in milliseconds without an
# emulator or an ARM toolchain.
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

# Levels come from maps/*.tmx through the same importer the ROM build uses, so the tests and the
# cartridge can never disagree about a level. gp_levels_test.cpp then proves each one finishable.
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(GENERATED_HEADER ${GENERATED_DIR}/include/gp_levels_generated.h)
set(GENERATED_SOURCE ${GENERATED_DIR}/src/gp_levels_generated.cpp)

file(GLOB MAP_FILES CONFIGURE_DEPENDS ${GAME_ROOT}/maps/*.tmx)

add_custom_command(
    OUTPUT ${GENERATED_HEADER} ${GENERATED_SOURCE}
    COMMAND ${Python3_EXECUTABLE} ${GAME_ROOT}/tools/gp_import_maps.py
            --maps ${GAME_ROOT}/maps
            --include ${GAME_ROOT}/include
            --out ${GENERATED_DIR}
    DEPENDS ${MAP_FILES}
            ${GAME_ROOT}/tools/gp_import_maps.py
            ${GAME_ROOT}/include/gp_level_data.h
    COMMENT "Importing Tiled maps"
    VERBATIM)

add_executable(gp_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_tests_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_collision_map_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_level_state_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_levels_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_signature_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_solver_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_trail_test.cpp
    # The solver checks that a level can be finished. CI runs it over every map; the ROM never
    # carries it, which is why it lives here rather than in src/.
    ${CMAKE_CURRENT_SOURCE_DIR}/gp_solver.cpp
    ${GENERATED_SOURCE}
    ${GAME_ROOT}/src/gp_collision_map.cpp
    ${GAME_ROOT}/src/gp_level_state.cpp
    ${GAME_ROOT}/src/gp_signature.cpp
    ${GAME_ROOT}/src/gp_trail.cpp)

target_include_directories(gp_tests PRIVATE
    ${GAME_ROOT}/include
    ${GENERATED_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(gp_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(gp_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME gp_tests COMMAND gp_tests)

# The importer has its own tests, and they need no compiler at all.
add_test(NAME gp_import_maps
         COMMAND ${Python3_EXECUTABLE} ${GAME_ROOT}/tools/gp_import_maps_test.py)
