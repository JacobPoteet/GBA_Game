/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_board.h"

namespace gp
{

namespace
{
/**
 * xorshift32.
 *
 * Butano ships bn::random, but board::fill has to stay usable from the host unit tests, so
 * it carries its own generator instead of reaching into the engine.
 */
[[nodiscard]] uint32_t next_random(uint32_t& state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}
} // namespace

board::board()
{
    clear();
}

tile_kind board::tile(int column, int row) const
{
    if (!contains(column, row))
    {
        return tile_kind::empty;
    }

    return _tiles[_index(column, row)];
}

void board::set_tile(int column, int row, tile_kind kind)
{
    if (!contains(column, row))
    {
        return;
    }

    _tiles[_index(column, row)] = kind;
}

void board::clear()
{
    for (int index = 0; index < cell_count; ++index)
    {
        _tiles[index] = tile_kind::empty;
    }
}

void board::fill(uint32_t seed)
{
    // A zero state would make xorshift32 emit zeroes forever.
    uint32_t state = seed ? seed : 1;

    for (int index = 0; index < cell_count; ++index)
    {
        _tiles[index] = tile_kind_from_graphics_index(int(next_random(state) % tile_kind_count));
    }
}

void board::swap_tiles(int first_column, int first_row, int second_column, int second_row)
{
    if (!contains(first_column, first_row) || !contains(second_column, second_row))
    {
        return;
    }

    tile_kind first = _tiles[_index(first_column, first_row)];
    _tiles[_index(first_column, first_row)] = _tiles[_index(second_column, second_row)];
    _tiles[_index(second_column, second_row)] = first;
}

int board::count(tile_kind kind) const
{
    int result = 0;

    for (int index = 0; index < cell_count; ++index)
    {
        if (_tiles[index] == kind)
        {
            ++result;
        }
    }

    return result;
}

bool board::full() const
{
    return count(tile_kind::empty) == 0;
}

} // namespace gp
