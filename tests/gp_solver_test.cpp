/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_level_fixture.h"
#include "gp_solver.h"

namespace
{

gp_fixture::level_builder make_room(int columns, int rows)
{
    gp_fixture::level_builder builder(columns, rows);

    for (int x = 0; x < columns; ++x)
    {
        builder.wall(x, 0);
        builder.wall(x, rows - 1);
    }

    for (int y = 0; y < rows; ++y)
    {
        builder.wall(0, y);
        builder.wall(columns - 1, y);
    }

    builder.start(1, 1);
    return builder;
}

} // namespace

GP_TEST(a_level_with_nobody_in_it_is_already_solved)
{
    gp_fixture::level_builder builder = make_room(6, 6);
    gp::level_spec spec = builder.spec();
    GP_CHECK(gp::solve(spec).solvable);
}

GP_TEST(one_pair_in_an_open_room_is_solvable)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{5, 1}, 0, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();

    gp::solve_result result = gp::solve(spec);
    GP_CHECK(result.solvable);
    GP_CHECK_EQ(int(result.steps.size()), 1);
}

GP_TEST(an_odd_number_of_characters_can_never_be_paired_off)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{4, 1}, 0, 0, gp::role::responder);
    builder.add(gp::tile_point{6, 1}, 0, 1, gp::role::caller);
    gp::level_spec spec = builder.spec();

    GP_CHECK(!gp::solve(spec).solvable);
}

GP_TEST(two_callers_alone_together_is_unsolvable)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{5, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();

    GP_CHECK(!gp::solve(spec).solvable);
}

GP_TEST(a_pair_sealed_behind_a_wall_is_unsolvable)
{
    gp_fixture::level_builder builder = make_room(9, 5);
    builder.wall(4, 1);
    builder.wall(4, 2);
    builder.wall(4, 3);

    builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::responder);
    builder.add(gp::tile_point{5, 1}, 1, 0, gp::role::caller);
    builder.add(gp::tile_point{7, 3}, 1, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();

    GP_CHECK(!gp::solve(spec).solvable);
}

GP_TEST(a_blocker_in_the_doorway_forces_the_order_of_play)
{
    gp_fixture::level_builder builder = make_room(9, 5);
    builder.wall(4, 1);
    builder.wall(4, 3);

    // The doorway pair. Both stand on the player's side of the wall.
    int door = builder.add(gp::tile_point{4, 2}, 0, 0, gp::role::caller, true);
    int partner = builder.add(gp::tile_point{3, 3}, 0, 0, gp::role::responder);

    // The far pair, out of reach until the doorway clears.
    builder.add(gp::tile_point{5, 1}, 1, 0, gp::role::caller);
    builder.add(gp::tile_point{7, 3}, 1, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();

    gp::solve_result result = gp::solve(spec);
    GP_CHECK(result.solvable);
    GP_CHECK_EQ(int(result.steps.size()), 2);

    if (result.steps.size() == 2)
    {
        bool door_first = (result.steps[0].first == door && result.steps[0].second == partner) ||
                          (result.steps[0].first == partner && result.steps[0].second == door);
        GP_CHECK(door_first);
    }
}

GP_TEST(carrying_someone_opens_the_tile_they_were_standing_on)
{
    // A closet at 5,2 whose only door is 4,2. The character in the doorway is the partner of the
    // character shut inside, so the level is solvable only because connecting empties the doorway.
    gp_fixture::level_builder builder = make_room(7, 5);
    builder.wall(4, 1);
    builder.wall(4, 3);
    builder.wall(5, 1);
    builder.wall(5, 3);

    builder.add(gp::tile_point{4, 2}, 0, 0, gp::role::caller, true);
    builder.add(gp::tile_point{5, 2}, 0, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();

    GP_CHECK(gp::solve(spec).solvable);
}

GP_TEST(matching_the_doorway_pair_opens_the_closet_for_good)
{
    // The doorway character belongs to a different family than the one shut in the closet, so the
    // player has to pair the doorway off first. That empties the tile permanently, which is what
    // puts the closet in reach.
    gp_fixture::level_builder builder = make_room(7, 5);
    builder.wall(4, 1);
    builder.wall(4, 3);
    builder.wall(5, 1);
    builder.wall(5, 3);

    int door = builder.add(gp::tile_point{4, 2}, 0, 1, gp::role::caller, true);
    int shut_in = builder.add(gp::tile_point{5, 2}, 0, 0, gp::role::responder);
    int door_partner = builder.add(gp::tile_point{2, 1}, 0, 1, gp::role::responder);
    int outside = builder.add(gp::tile_point{3, 3}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();

    gp::solve_result result = gp::solve(spec);
    GP_CHECK(result.solvable);
    GP_CHECK_EQ(int(result.steps.size()), 2);

    if (result.steps.size() == 2)
    {
        bool doorway_first = (result.steps[0].first == door && result.steps[0].second == door_partner) ||
                             (result.steps[0].first == door_partner && result.steps[0].second == door);
        GP_CHECK(doorway_first);

        bool closet_last = (result.steps[1].first == shut_in && result.steps[1].second == outside) ||
                           (result.steps[1].first == outside && result.steps[1].second == shut_in);
        GP_CHECK(closet_last);
    }
}

GP_TEST(every_returned_step_names_a_true_pair)
{
    gp_fixture::level_builder builder = make_room(9, 5);
    builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{3, 3}, 0, 0, gp::role::responder);
    builder.add(gp::tile_point{5, 1}, 1, 2, gp::role::caller);
    builder.add(gp::tile_point{7, 3}, 1, 2, gp::role::responder);
    gp::level_spec spec = builder.spec();

    gp::solve_result result = gp::solve(spec);
    GP_CHECK(result.solvable);

    for (const gp::solve_step& step : result.steps)
    {
        GP_CHECK(gp::matches(spec.characters[step.first].sig, spec.characters[step.second].sig));
    }
}
