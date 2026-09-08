/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

/*
 * Runs over the real maps, not fixtures. tools/gp_import_maps.py turns every .tmx in maps/ into
 * gp_levels_generated.cpp before this build starts, so a level that cannot be finished fails CI
 * the same way a broken function does.
 */

#include "gp_test_framework.h"

#include "gp_collision_map.h"
#include "gp_level_state.h"
#include "gp_levels_generated.h"
#include "gp_solver.h"

namespace
{

[[nodiscard]] bool held_for_the_player(const gp::level_spec& spec, int index)
{
    return gp::matches(gp::player_signature, spec.characters[index].sig);
}

[[nodiscard]] int player_match_count(const gp::level_spec& spec)
{
    int count = 0;

    for (int index = 0; index < spec.character_count; ++index)
    {
        if (held_for_the_player(spec, index))
        {
            ++count;
        }
    }

    return count;
}

[[nodiscard]] bool has_partner(const gp::level_spec& spec, int index)
{
    for (int other = 0; other < spec.character_count; ++other)
    {
        if (other != index && gp::matches(spec.characters[index].sig, spec.characters[other].sig))
        {
            return true;
        }
    }

    return false;
}

} // namespace

GP_TEST(the_generated_level_table_is_not_empty)
{
    GP_CHECK(gp::level_count >= 1);
}

GP_TEST(every_level_can_be_finished)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];
        gp::solve_result result = gp::solve(spec);

        // A failure here names a map that has to change, not a function.
        GP_CHECK(result.solvable);
        GP_CHECK_EQ(int(result.steps.size()) * 2, spec.character_count - player_match_count(spec));
    }
}

GP_TEST(every_solution_step_names_a_true_pair)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];

        for (const gp::solve_step& step : gp::solve(spec).steps)
        {
            GP_CHECK(gp::matches(spec.characters[step.first].sig, spec.characters[step.second].sig));
        }
    }
}

GP_TEST(every_level_holds_an_even_number_of_characters_to_pair_off)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];
        GP_CHECK(spec.character_count > 0);
        GP_CHECK(spec.character_count <= gp::max_characters);

        // The one the level holds for the player is not a puzzle piece, so they are left out.
        GP_CHECK_EQ((spec.character_count - player_match_count(spec)) % 2, 0);
    }
}

GP_TEST(nobody_matches_the_player_until_the_last_level)
{
    for (int index = 0; index < gp::level_count - 1; ++index)
    {
        GP_CHECK_EQ(player_match_count(gp::levels[index]), 0);
    }
}

GP_TEST(the_last_level_holds_exactly_one_person_for_the_player)
{
    GP_CHECK(gp::level_count >= 1);
    GP_CHECK_EQ(player_match_count(gp::levels[gp::level_count - 1]), 1);
}

GP_TEST(nobody_in_a_level_is_left_without_a_partner)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];

        for (int character = 0; character < spec.character_count; ++character)
        {
            // The player's own match is answered by the player, not by anybody in the level.
            if (held_for_the_player(spec, character))
            {
                GP_CHECK(!has_partner(spec, character));
                continue;
            }

            GP_CHECK(has_partner(spec, character));
        }
    }
}

GP_TEST(nobody_is_standing_inside_a_wall)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];
        gp::collision_map terrain(spec.columns, spec.rows, spec.collision_bits, spec.collision_byte_count);

        GP_CHECK(!terrain.blocked(spec.player_start));

        for (int character = 0; character < spec.character_count; ++character)
        {
            GP_CHECK(!terrain.blocked(spec.characters[character].position));
        }
    }
}

GP_TEST(every_flee_route_is_walkable_and_in_range)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];
        gp::collision_map terrain(spec.columns, spec.rows, spec.collision_bits, spec.collision_byte_count);

        for (int character = 0; character < spec.character_count; ++character)
        {
            int route = spec.characters[character].flee_route;
            GP_CHECK(route >= gp::no_route && route < spec.route_count);
        }

        for (int route = 0; route < spec.route_count; ++route)
        {
            const gp::route& path = spec.routes[route];
            GP_CHECK(path.length > 0);
            GP_CHECK(path.length <= gp::max_route_points);

            for (int point = 0; point < int(path.length); ++point)
            {
                GP_CHECK(!terrain.blocked(path.points[point]));
            }
        }
    }
}

GP_TEST(every_level_loads_into_the_simulation)
{
    for (int index = 0; index < gp::level_count; ++index)
    {
        const gp::level_spec& spec = gp::levels[index];
        gp::level_state state(spec);

        GP_CHECK_EQ(state.character_count(), spec.character_count);
        GP_CHECK(state.player_position() == spec.player_start);
        GP_CHECK_EQ(state.follower(), gp::no_character);
        GP_CHECK(!state.complete());
        GP_CHECK(!state.solid(state.player_position()));
    }
}

GP_TEST(level_01_gates_its_far_pair_behind_the_doorway)
{
    // The shape the first level is meant to teach: two pairs reachable from the start, and one
    // that stays out of reach until the character in the doorway walks off.
    GP_CHECK(gp::level_count >= 1);

    const gp::level_spec& spec = gp::levels[0];
    gp::solve_result result = gp::solve(spec);
    GP_CHECK(result.solvable);
    GP_CHECK_EQ(int(result.steps.size()), 3);

    int blockers = 0;

    for (int index = 0; index < spec.character_count; ++index)
    {
        if (spec.characters[index].is_blocker)
        {
            ++blockers;
        }
    }

    GP_CHECK_EQ(blockers, 1);
}
