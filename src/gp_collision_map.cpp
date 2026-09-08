/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_collision_map.h"

namespace gp
{

collision_map::collision_map(int columns, int rows, const uint8_t* bits, int bit_byte_count)
{
    _columns = columns < 0 ? 0 : (columns > max_columns ? max_columns : columns);
    _rows = rows < 0 ? 0 : (rows > max_rows ? max_rows : rows);

    if (!bits || bit_byte_count <= 0)
    {
        return;
    }

    int copied = bit_byte_count > byte_count ? byte_count : bit_byte_count;

    for (int index = 0; index < copied; ++index)
    {
        _bits[index] = bits[index];
    }
}

bool collision_map::blocked(tile_point point) const
{
    if (!contains(point))
    {
        return true;
    }

    int bit = _index(point);
    return (_bits[bit / 8] & uint8_t(1 << (bit % 8))) != 0;
}

void collision_map::set_blocked(tile_point point, bool value)
{
    if (!contains(point))
    {
        return;
    }

    int bit = _index(point);
    uint8_t mask = uint8_t(1 << (bit % 8));

    if (value)
    {
        _bits[bit / 8] = uint8_t(_bits[bit / 8] | mask);
    }
    else
    {
        _bits[bit / 8] = uint8_t(_bits[bit / 8] & uint8_t(~mask));
    }
}

} // namespace gp
