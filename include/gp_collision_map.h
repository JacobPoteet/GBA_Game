/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_COLLISION_MAP_H
#define GP_COLLISION_MAP_H

#include <cstdint>

#include "gp_tile_point.h"

namespace gp
{

/**
 * One bit per tile: set means solid.
 *
 * Sized for the largest level a Butano regular background holds without `big_map`, which is
 * 512x512 pixels of 16 pixel tiles. That costs 128 bytes, so the map is copied by value without
 * thought.
 */
class collision_map
{

public:
    static constexpr int max_columns = 32;
    static constexpr int max_rows = 32;
    static constexpr int max_tiles = max_columns * max_rows;
    static constexpr int byte_count = max_tiles / 8;

    collision_map() = default;

    /**
     * Builds a map from packed bits, least significant bit first, in row-major order. Bits past
     * `byte_count` bytes are ignored, and missing bytes read as open.
     */
    collision_map(int columns, int rows, const uint8_t* bits, int bit_byte_count);

    [[nodiscard]] int columns() const
    {
        return _columns;
    }

    [[nodiscard]] int rows() const
    {
        return _rows;
    }

    /** Anything off the grid is solid, so callers never bounds check before asking. */
    [[nodiscard]] bool blocked(tile_point point) const;

    /** Ignored when the point is off the grid. */
    void set_blocked(tile_point point, bool value);

    [[nodiscard]] bool contains(tile_point point) const
    {
        return point.x >= 0 && point.x < _columns && point.y >= 0 && point.y < _rows;
    }

private:
    uint8_t _bits[byte_count] = {};
    int _columns = 0;
    int _rows = 0;

    [[nodiscard]] int _index(tile_point point) const
    {
        return (point.y * _columns) + point.x;
    }
};

} // namespace gp

#endif
