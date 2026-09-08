/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_TILE_POINT_H
#define GP_TILE_POINT_H

namespace gp
{

/**
 * Integer tile coordinates. The whole simulation works in tiles; the Butano layer converts to
 * pixels when it draws.
 */
struct tile_point
{
    int x = 0;
    int y = 0;

    [[nodiscard]] friend constexpr bool operator==(tile_point a, tile_point b) = default;
};

[[nodiscard]] constexpr tile_point operator+(tile_point a, tile_point b)
{
    return tile_point{a.x + b.x, a.y + b.y};
}

[[nodiscard]] constexpr tile_point operator-(tile_point a, tile_point b)
{
    return tile_point{a.x - b.x, a.y - b.y};
}

/**
 * Manhattan distance. Movement is four-way, so this is the true step count between two tiles on an
 * empty grid.
 */
[[nodiscard]] constexpr int manhattan_distance(tile_point a, tile_point b)
{
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
}

[[nodiscard]] constexpr bool adjacent(tile_point a, tile_point b)
{
    return manhattan_distance(a, b) == 1;
}

} // namespace gp

#endif
