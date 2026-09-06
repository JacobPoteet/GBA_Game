/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_cursor.h"

GP_TEST(cursor_starts_inside_the_board)
{
    gp::cursor cursor;

    GP_CHECK(gp::board::contains(cursor.column(), cursor.row()));
}

GP_TEST(cursor_moves_by_one_cell)
{
    gp::cursor cursor;
    cursor.set_position(2, 2);

    GP_CHECK(cursor.move(1, 0));
    GP_CHECK_EQ(cursor.column(), 3);
    GP_CHECK_EQ(cursor.row(), 2);

    GP_CHECK(cursor.move(0, 1));
    GP_CHECK_EQ(cursor.row(), 3);
}

GP_TEST(cursor_clamps_at_the_edges_instead_of_wrapping)
{
    gp::cursor cursor;
    cursor.set_position(0, 0);

    GP_CHECK(!cursor.move(-1, 0));
    GP_CHECK_EQ(cursor.column(), 0);

    GP_CHECK(!cursor.move(0, -1));
    GP_CHECK_EQ(cursor.row(), 0);

    cursor.set_position(gp::board::columns - 1, gp::board::rows - 1);

    GP_CHECK(!cursor.move(1, 0));
    GP_CHECK_EQ(cursor.column(), gp::board::columns - 1);

    GP_CHECK(!cursor.move(0, 1));
    GP_CHECK_EQ(cursor.row(), gp::board::rows - 1);
}

GP_TEST(cursor_set_position_clamps_out_of_range_input)
{
    gp::cursor cursor;
    cursor.set_position(-10, 999);

    GP_CHECK_EQ(cursor.column(), 0);
    GP_CHECK_EQ(cursor.row(), gp::board::rows - 1);
}

GP_TEST(cursor_move_reports_whether_it_moved)
{
    gp::cursor cursor;
    cursor.set_position(1, 1);

    GP_CHECK(!cursor.move(0, 0));
    GP_CHECK(cursor.move(2, 2));
    GP_CHECK_EQ(cursor.column(), 3);
    GP_CHECK_EQ(cursor.row(), 3);
}
