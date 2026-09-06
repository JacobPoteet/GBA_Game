/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_BOARD_H
#define GP_BOARD_H

#include <cstdint>

#include "gp_tile_kind.h"

namespace gp
{

/**
 * The puzzle grid.
 *
 * Pure logic with no Butano dependency and no dynamic allocation, so the host unit tests in
 * tests/ can exercise it directly. Out of bounds reads return tile_kind::empty and out of bounds
 * writes are ignored, which keeps callers free of bounds checks and keeps the behaviour testable.
 */
class board
{

public:
    static constexpr int columns = 6;
    static constexpr int rows = 6;
    static constexpr int cell_count = columns * rows;

    board();

    [[nodiscard]] static constexpr bool contains(int column, int row)
    {
        return column >= 0 && column < columns && row >= 0 && row < rows;
    }

    [[nodiscard]] tile_kind tile(int column, int row) const;

    void set_tile(int column, int row, tile_kind kind);

    void clear();

    /// Fills every cell with a non-empty kind, deterministically derived from `seed`.
    void fill(uint32_t seed);

    void swap_tiles(int first_column, int first_row, int second_column, int second_row);

    [[nodiscard]] int count(tile_kind kind) const;

    [[nodiscard]] bool full() const;

private:
    tile_kind _tiles[cell_count];

    [[nodiscard]] static constexpr int _index(int column, int row)
    {
        return (row * columns) + column;
    }
};

} // namespace gp

#endif
