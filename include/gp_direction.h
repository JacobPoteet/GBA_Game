/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_DIRECTION_H
#define GP_DIRECTION_H

#include <cstdint>

#include "gp_tile_point.h"

namespace gp
{

enum class direction : uint8_t
{
    up = 0,
    down,
    left,
    right
};

constexpr int direction_count = 4;

[[nodiscard]] constexpr tile_point direction_delta(direction dir)
{
    switch (dir)
    {

    case direction::up:
        return tile_point{0, -1};

    case direction::down:
        return tile_point{0, 1};

    case direction::left:
        return tile_point{-1, 0};

    case direction::right:
        return tile_point{1, 0};
    }

    return tile_point{0, 0};
}

[[nodiscard]] constexpr direction opposite(direction dir)
{
    switch (dir)
    {

    case direction::up:
        return direction::down;

    case direction::down:
        return direction::up;

    case direction::left:
        return direction::right;

    case direction::right:
        return direction::left;
    }

    return dir;
}

/**
 * The direction that steps from `from` to `to`, for adjacent tiles. Returns `direction::down` for
 * any other pair, so callers that only ever pass neighbours do not have to handle a failure case.
 */
[[nodiscard]] constexpr direction direction_towards(tile_point from, tile_point to)
{
    tile_point delta = to - from;

    if (delta.x > 0)
    {
        return direction::right;
    }

    if (delta.x < 0)
    {
        return direction::left;
    }

    if (delta.y < 0)
    {
        return direction::up;
    }

    return direction::down;
}

} // namespace gp

#endif
