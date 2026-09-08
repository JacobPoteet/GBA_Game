/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_collision_map.h"

GP_TEST(a_default_map_has_no_room_in_it)
{
    gp::collision_map map;
    GP_CHECK_EQ(map.columns(), 0);
    GP_CHECK_EQ(map.rows(), 0);
    GP_CHECK(map.blocked(gp::tile_point{0, 0}));
}

GP_TEST(a_map_built_from_no_bits_is_open)
{
    gp::collision_map map(4, 4, nullptr, 0);
    GP_CHECK_EQ(map.columns(), 4);
    GP_CHECK_EQ(map.rows(), 4);
    GP_CHECK(!map.blocked(gp::tile_point{0, 0}));
    GP_CHECK(!map.blocked(gp::tile_point{3, 3}));
}

GP_TEST(everything_outside_the_grid_is_solid)
{
    gp::collision_map map(4, 4, nullptr, 0);
    GP_CHECK(map.blocked(gp::tile_point{-1, 0}));
    GP_CHECK(map.blocked(gp::tile_point{0, -1}));
    GP_CHECK(map.blocked(gp::tile_point{4, 0}));
    GP_CHECK(map.blocked(gp::tile_point{0, 4}));
}

GP_TEST(setting_a_tile_solid_reads_back)
{
    gp::collision_map map(8, 8, nullptr, 0);
    map.set_blocked(gp::tile_point{3, 5}, true);
    GP_CHECK(map.blocked(gp::tile_point{3, 5}));
    GP_CHECK(!map.blocked(gp::tile_point{4, 5}));
    GP_CHECK(!map.blocked(gp::tile_point{3, 6}));
}

GP_TEST(clearing_a_tile_reads_back)
{
    gp::collision_map map(8, 8, nullptr, 0);
    map.set_blocked(gp::tile_point{3, 5}, true);
    map.set_blocked(gp::tile_point{3, 5}, false);
    GP_CHECK(!map.blocked(gp::tile_point{3, 5}));
}

GP_TEST(writing_outside_the_grid_is_ignored)
{
    gp::collision_map map(4, 4, nullptr, 0);
    map.set_blocked(gp::tile_point{9, 9}, true);
    GP_CHECK(!map.blocked(gp::tile_point{1, 1}));
}

GP_TEST(packed_bits_unpack_row_major_least_significant_bit_first)
{
    // An 8x2 grid. The first byte is row 0, so bits 0 and 7 are its two ends.
    const uint8_t bits[2] = {0x81, 0x02};
    gp::collision_map map(8, 2, bits, 2);

    GP_CHECK(map.blocked(gp::tile_point{0, 0}));
    GP_CHECK(!map.blocked(gp::tile_point{1, 0}));
    GP_CHECK(map.blocked(gp::tile_point{7, 0}));
    GP_CHECK(!map.blocked(gp::tile_point{0, 1}));
    GP_CHECK(map.blocked(gp::tile_point{1, 1}));
}

GP_TEST(a_short_bit_array_leaves_the_rest_open)
{
    const uint8_t bits[1] = {0x01};
    gp::collision_map map(8, 2, bits, 1);
    GP_CHECK(map.blocked(gp::tile_point{0, 0}));
    GP_CHECK(!map.blocked(gp::tile_point{4, 1}));
}

GP_TEST(a_grid_larger_than_the_maximum_is_clamped)
{
    gp::collision_map map(999, 999, nullptr, 0);
    GP_CHECK_EQ(map.columns(), gp::collision_map::max_columns);
    GP_CHECK_EQ(map.rows(), gp::collision_map::max_rows);
}

GP_TEST(contains_agrees_with_the_grid_bounds)
{
    gp::collision_map map(5, 3, nullptr, 0);
    GP_CHECK(map.contains(gp::tile_point{4, 2}));
    GP_CHECK(!map.contains(gp::tile_point{5, 2}));
    GP_CHECK(!map.contains(gp::tile_point{4, 3}));
}
