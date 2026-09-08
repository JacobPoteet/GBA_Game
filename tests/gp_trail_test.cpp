/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_trail.h"

GP_TEST(a_reset_trail_holds_one_tile)
{
    gp::trail path;
    path.reset(gp::tile_point{4, 7});
    GP_CHECK_EQ(path.size(), 1);
    GP_CHECK(path.at(0) == (gp::tile_point{4, 7}));
}

GP_TEST(a_fresh_follower_stands_on_the_player)
{
    gp::trail path;
    path.reset(gp::tile_point{4, 7});
    GP_CHECK(path.at(1) == (gp::tile_point{4, 7}));
}

GP_TEST(one_step_puts_the_follower_where_the_player_was)
{
    gp::trail path;
    path.reset(gp::tile_point{0, 0});
    path.push(gp::tile_point{1, 0});
    GP_CHECK(path.at(0) == (gp::tile_point{1, 0}));
    GP_CHECK(path.at(1) == (gp::tile_point{0, 0}));
}

GP_TEST(the_trail_retraces_the_walked_path_rather_than_the_straight_line)
{
    gp::trail path;
    path.reset(gp::tile_point{0, 0});
    path.push(gp::tile_point{1, 0});
    path.push(gp::tile_point{1, 1});
    path.push(gp::tile_point{2, 1});

    GP_CHECK(path.at(1) == (gp::tile_point{1, 1}));
    GP_CHECK(path.at(2) == (gp::tile_point{1, 0}));
    GP_CHECK(path.at(3) == (gp::tile_point{0, 0}));
}

GP_TEST(reading_past_the_history_clamps_to_the_oldest_tile)
{
    gp::trail path;
    path.reset(gp::tile_point{5, 5});
    path.push(gp::tile_point{6, 5});
    GP_CHECK(path.at(9) == (gp::tile_point{5, 5}));
}

GP_TEST(a_negative_step_reads_the_newest_tile)
{
    gp::trail path;
    path.reset(gp::tile_point{5, 5});
    path.push(gp::tile_point{6, 5});
    GP_CHECK(path.at(-3) == (gp::tile_point{6, 5}));
}

GP_TEST(the_trail_stops_growing_at_capacity)
{
    gp::trail path;
    path.reset(gp::tile_point{0, 0});

    for (int step = 1; step < gp::trail::capacity + 10; ++step)
    {
        path.push(gp::tile_point{step, 0});
    }

    GP_CHECK_EQ(path.size(), gp::trail::capacity);
}

GP_TEST(the_oldest_tiles_fall_off_once_the_trail_wraps)
{
    gp::trail path;
    path.reset(gp::tile_point{0, 0});

    for (int step = 1; step <= gp::trail::capacity; ++step)
    {
        path.push(gp::tile_point{step, 0});
    }

    GP_CHECK(path.at(0) == (gp::tile_point{gp::trail::capacity, 0}));
    GP_CHECK(path.at(gp::trail::capacity - 1) == (gp::tile_point{1, 0}));
}

GP_TEST(resetting_a_wrapped_trail_clears_the_history)
{
    gp::trail path;
    path.reset(gp::tile_point{0, 0});

    for (int step = 1; step <= gp::trail::capacity; ++step)
    {
        path.push(gp::tile_point{step, 0});
    }

    path.reset(gp::tile_point{9, 9});
    GP_CHECK_EQ(path.size(), 1);
    GP_CHECK(path.at(3) == (gp::tile_point{9, 9}));
}
