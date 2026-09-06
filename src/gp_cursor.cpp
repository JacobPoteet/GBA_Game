/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_cursor.h"

namespace gp
{

namespace
{
[[nodiscard]] int clamp(int value, int minimum, int maximum)
{
    if (value < minimum)
    {
        return minimum;
    }

    if (value > maximum)
    {
        return maximum;
    }

    return value;
}
} // namespace

void cursor::set_position(int column, int row)
{
    _column = clamp(column, 0, board::columns - 1);
    _row = clamp(row, 0, board::rows - 1);
}

bool cursor::move(int column_delta, int row_delta)
{
    int previous_column = _column;
    int previous_row = _row;
    set_position(_column + column_delta, _row + row_delta);
    return _column != previous_column || _row != previous_row;
}

} // namespace gp
