/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_CURSOR_H
#define GP_CURSOR_H

#include "gp_board.h"

namespace gp
{

/**
 * The player's selection on the board.
 *
 * Like gp::board this is Butano free and host testable; the scene layer maps it to a sprite.
 * Movement clamps at the edges rather than wrapping.
 */
class cursor
{

public:
    cursor() = default;

    [[nodiscard]] int column() const
    {
        return _column;
    }

    [[nodiscard]] int row() const
    {
        return _row;
    }

    /// Clamps into the board; out of range values snap to the nearest valid cell.
    void set_position(int column, int row);

    /// Returns true if the cursor actually moved.
    bool move(int column_delta, int row_delta);

private:
    int _column = board::columns / 2;
    int _row = board::rows / 2;
};

} // namespace gp

#endif
