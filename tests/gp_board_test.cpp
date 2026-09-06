/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_board.h"

GP_TEST(board_starts_empty)
{
    gp::board board;

    GP_CHECK_EQ(board.count(gp::tile_kind::empty), gp::board::cell_count);
    GP_CHECK(!board.full());
    GP_CHECK_EQ(board.tile(0, 0), gp::tile_kind::empty);
}

GP_TEST(board_contains_rejects_out_of_range)
{
    GP_CHECK(gp::board::contains(0, 0));
    GP_CHECK(gp::board::contains(gp::board::columns - 1, gp::board::rows - 1));
    GP_CHECK(!gp::board::contains(-1, 0));
    GP_CHECK(!gp::board::contains(0, -1));
    GP_CHECK(!gp::board::contains(gp::board::columns, 0));
    GP_CHECK(!gp::board::contains(0, gp::board::rows));
}

GP_TEST(board_set_and_get_tile)
{
    gp::board board;
    board.set_tile(2, 3, gp::tile_kind::blue);

    GP_CHECK_EQ(board.tile(2, 3), gp::tile_kind::blue);
    GP_CHECK_EQ(board.tile(3, 2), gp::tile_kind::empty);
    GP_CHECK_EQ(board.count(gp::tile_kind::blue), 1);
}

GP_TEST(board_out_of_bounds_access_is_a_no_op)
{
    gp::board board;
    board.set_tile(-1, 0, gp::tile_kind::red);
    board.set_tile(0, gp::board::rows, gp::tile_kind::red);

    GP_CHECK_EQ(board.count(gp::tile_kind::red), 0);
    GP_CHECK_EQ(board.tile(-1, 0), gp::tile_kind::empty);
    GP_CHECK_EQ(board.tile(gp::board::columns, 0), gp::tile_kind::empty);
}

GP_TEST(board_fill_leaves_no_empty_cells)
{
    gp::board board;
    board.fill(12345);

    GP_CHECK(board.full());
    GP_CHECK_EQ(board.count(gp::tile_kind::empty), 0);

    int total = 0;

    for (int index = 0; index < gp::tile_kind_count; ++index)
    {
        total += board.count(gp::tile_kind_from_graphics_index(index));
    }

    GP_CHECK_EQ(total, gp::board::cell_count);
}

GP_TEST(board_fill_is_deterministic_for_a_seed)
{
    gp::board first;
    gp::board second;
    first.fill(0xABCDEF);
    second.fill(0xABCDEF);

    for (int row = 0; row < gp::board::rows; ++row)
    {
        for (int column = 0; column < gp::board::columns; ++column)
        {
            GP_CHECK_EQ(first.tile(column, row), second.tile(column, row));
        }
    }
}

GP_TEST(board_fill_survives_a_zero_seed)
{
    gp::board board;
    board.fill(0);

    GP_CHECK(board.full());
}

GP_TEST(board_swap_tiles)
{
    gp::board board;
    board.set_tile(0, 0, gp::tile_kind::red);
    board.set_tile(1, 0, gp::tile_kind::green);
    board.swap_tiles(0, 0, 1, 0);

    GP_CHECK_EQ(board.tile(0, 0), gp::tile_kind::green);
    GP_CHECK_EQ(board.tile(1, 0), gp::tile_kind::red);
}

GP_TEST(board_swap_out_of_bounds_changes_nothing)
{
    gp::board board;
    board.set_tile(0, 0, gp::tile_kind::red);
    board.swap_tiles(0, 0, -5, 0);

    GP_CHECK_EQ(board.tile(0, 0), gp::tile_kind::red);
}

GP_TEST(board_clear_resets_every_cell)
{
    gp::board board;
    board.fill(7);
    board.clear();

    GP_CHECK_EQ(board.count(gp::tile_kind::empty), gp::board::cell_count);
}

GP_TEST(tile_kind_graphics_index_round_trips)
{
    for (int index = 0; index < gp::tile_kind_count; ++index)
    {
        gp::tile_kind kind = gp::tile_kind_from_graphics_index(index);

        GP_CHECK(!gp::is_empty(kind));
        GP_CHECK_EQ(gp::tile_kind_graphics_index(kind), index);
    }
}

GP_TEST(next_tile_kind_cycles_through_every_kind)
{
    gp::tile_kind kind = gp::tile_kind::red;

    for (int step = 0; step < gp::tile_kind_count; ++step)
    {
        GP_CHECK(!gp::is_empty(kind));
        kind = gp::next_tile_kind(kind);
    }

    GP_CHECK_EQ(kind, gp::tile_kind::red);
    GP_CHECK_EQ(gp::next_tile_kind(gp::tile_kind::empty), gp::tile_kind::red);
}
