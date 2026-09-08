/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_LEVEL_DATA_H
#define GP_LEVEL_DATA_H

#include <cstdint>

#include "gp_signature.h"
#include "gp_tile_point.h"

namespace gp
{

constexpr int max_characters = 16;
constexpr int max_routes = 16;
constexpr int max_route_points = 8;

/** Index into a level's character array. `no_character` means nobody. */
constexpr int no_character = -1;

/** Index into a level's route array. `no_route` means the character stays put when rejected. */
constexpr int no_route = -1;

/**
 * A path a rejected character walks away along, authored in the map editor rather than pathfound.
 * The last point is where they settle and wait to be found again.
 */
struct route
{
    tile_point points[max_route_points] = {};
    uint8_t length = 0;
};

/**
 * One character as the map editor placed them.
 */
struct character_spec
{
    tile_point position;
    signature sig;

    /** Solid while present. Matching them is what opens the tile they stand on. */
    bool is_blocker = false;

    int8_t flee_route = int8_t(no_route);
};

/**
 * Everything the simulation needs to run a level. Emitted by `tools/gp_import_maps.py` as static
 * data, with no Butano types anywhere in it, so the host test build reads the same levels the ROM
 * does.
 */
struct level_spec
{
    const char* name = "";
    int columns = 0;
    int rows = 0;

    /** Packed collision bits, least significant bit first, row-major. */
    const uint8_t* collision_bits = nullptr;
    int collision_byte_count = 0;

    tile_point player_start;

    const character_spec* characters = nullptr;
    int character_count = 0;

    const route* routes = nullptr;
    int route_count = 0;
};

} // namespace gp

#endif
