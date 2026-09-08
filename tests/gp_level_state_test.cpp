/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_level_fixture.h"
#include "gp_level_state.h"

namespace
{

/** An open room with a solid border, and the player standing in the top left corner. */
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

/** Turns to face `dir` and then walks `steps` tiles that way. */
void walk(gp::level_state& state, gp::direction dir, int steps)
{
    state.try_move(dir);

    for (int step = 0; step < steps; ++step)
    {
        state.try_move(dir);
    }
}

} // namespace

GP_TEST(a_new_level_places_the_player_and_leaves_them_empty_handed)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.player_position() == (gp::tile_point{1, 1}));
    GP_CHECK_EQ(state.character_count(), 1);
    GP_CHECK_EQ(state.follower(), gp::no_character);
    GP_CHECK(!state.complete());
}

GP_TEST(the_first_press_turns_and_the_second_walks)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.try_move(gp::direction::right));
    GP_CHECK(state.player_facing() == gp::direction::right);
    GP_CHECK(state.player_position() == (gp::tile_point{1, 1}));

    GP_CHECK(state.try_move(gp::direction::right));
    GP_CHECK(state.player_position() == (gp::tile_point{2, 1}));
}

GP_TEST(turning_to_the_way_you_already_face_reports_no_change)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    state.turn(gp::direction::right);
    GP_CHECK(!state.turn(gp::direction::right));
}

GP_TEST(a_wall_stops_the_player)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    state.try_move(gp::direction::left);
    GP_CHECK(!state.try_move(gp::direction::left));
    GP_CHECK(state.player_position() == (gp::tile_point{1, 1}));
}

GP_TEST(an_idle_blocker_stops_the_player)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller, true);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.solid(gp::tile_point{2, 1}));
    state.try_move(gp::direction::right);
    GP_CHECK(!state.try_move(gp::direction::right));
    GP_CHECK(state.player_position() == (gp::tile_point{1, 1}));
}

GP_TEST(everyone_standing_still_is_solid_whether_or_not_they_gate_a_route)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller, false);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.solid(gp::tile_point{2, 1}));
}

GP_TEST(a_follower_is_not_solid_so_walking_backwards_swaps_the_two_of_you)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    GP_CHECK(state.character_at(who).position == (gp::tile_point{1, 1}));
    GP_CHECK(!state.solid(gp::tile_point{1, 1}));

    walk(state, gp::direction::left, 1);
    GP_CHECK(state.player_position() == (gp::tile_point{1, 1}));
    GP_CHECK(state.character_at(who).position == (gp::tile_point{2, 1}));
}

GP_TEST(the_player_faces_whoever_stands_in_the_next_tile)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK_EQ(state.facing_character(), gp::no_character);
    state.turn(gp::direction::right);
    GP_CHECK_EQ(state.facing_character(), who);
}

GP_TEST(listening_queues_a_chirp_from_the_faced_character)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    state.turn(gp::direction::right);
    GP_CHECK(state.listen());
    GP_CHECK_EQ(state.event_count(), 1);
    GP_CHECK(state.event_at(0).kind == gp::event_kind::chirp);
    GP_CHECK_EQ(int(state.event_at(0).character), who);
}

GP_TEST(listening_at_nobody_queues_nothing)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(!state.listen());
    GP_CHECK_EQ(state.event_count(), 0);
}

GP_TEST(clearing_events_empties_the_queue)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{2, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    state.turn(gp::direction::right);
    state.listen();
    state.clear_events();
    GP_CHECK_EQ(state.event_count(), 0);
}

GP_TEST(connecting_attaches_the_character_behind_the_player)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    GP_CHECK(state.player_position() == (gp::tile_point{2, 1}));
    GP_CHECK(state.interact() == gp::interact_result::connected);
    GP_CHECK_EQ(state.follower(), who);
    GP_CHECK(state.character_at(who).state == gp::character_state::following);
    GP_CHECK(state.character_at(who).position == (gp::tile_point{1, 1}));
}

GP_TEST(a_follower_walks_the_tiles_the_player_walked)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();

    state.try_move(gp::direction::down);
    state.try_move(gp::direction::down);
    GP_CHECK(state.player_position() == (gp::tile_point{2, 2}));
    GP_CHECK(state.character_at(who).position == (gp::tile_point{2, 1}));
}

GP_TEST(turning_in_place_leaves_the_follower_where_it_stands)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    gp::tile_point before = state.character_at(who).position;

    state.turn(gp::direction::left);
    GP_CHECK(state.character_at(who).position == before);
}

GP_TEST(turning_around_faces_your_own_follower)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.turn(gp::direction::left);
    GP_CHECK_EQ(state.facing_character(), who);
}

GP_TEST(interacting_with_your_own_follower_lets_them_go)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int who = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.turn(gp::direction::left);

    GP_CHECK(state.interact() == gp::interact_result::released);
    GP_CHECK_EQ(state.follower(), gp::no_character);
    GP_CHECK(state.character_at(who).state == gp::character_state::idle);
}

GP_TEST(introducing_a_true_pair_sends_them_both_away)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int caller = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    int responder = builder.add(gp::tile_point{5, 1}, 0, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);

    GP_CHECK(state.player_position() == (gp::tile_point{4, 1}));
    GP_CHECK(state.interact() == gp::interact_result::matched);
    GP_CHECK(state.character_at(caller).state == gp::character_state::departed);
    GP_CHECK(state.character_at(responder).state == gp::character_state::departed);
    GP_CHECK_EQ(state.follower(), gp::no_character);
    GP_CHECK(state.complete());
}

GP_TEST(clearing_the_level_is_announced_once)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{5, 1}, 0, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);
    state.clear_events();
    state.interact();

    int cleared = 0;

    for (int index = 0; index < state.event_count(); ++index)
    {
        if (state.event_at(index).kind == gp::event_kind::level_cleared)
        {
            ++cleared;
        }
    }

    GP_CHECK_EQ(cleared, 1);
}

GP_TEST(the_same_clan_and_a_different_family_is_rejected)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int route = builder.add_route({gp::tile_point{3, 1}, gp::tile_point{3, 2}, gp::tile_point{3, 3}});
    int carried = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller, false, route);
    builder.add(gp::tile_point{5, 1}, 0, 1, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);

    GP_CHECK(state.interact() == gp::interact_result::rejected);
    GP_CHECK(state.character_at(carried).state == gp::character_state::fleeing);
    GP_CHECK_EQ(state.follower(), gp::no_character);
    GP_CHECK(!state.complete());
}

GP_TEST(a_rejected_character_walks_its_route_and_settles_at_the_end)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int route = builder.add_route({gp::tile_point{3, 1}, gp::tile_point{3, 2}, gp::tile_point{3, 3}});
    int carried = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller, false, route);
    builder.add(gp::tile_point{5, 1}, 0, 1, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);
    state.interact();

    state.tick();
    GP_CHECK(state.character_at(carried).position == (gp::tile_point{3, 2}));

    state.tick();
    GP_CHECK(state.character_at(carried).position == (gp::tile_point{3, 3}));

    state.tick();
    GP_CHECK(state.character_at(carried).state == gp::character_state::idle);
    GP_CHECK(state.character_at(carried).position == (gp::tile_point{3, 3}));
}

GP_TEST(a_rejected_character_with_no_route_stops_where_they_stand)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int carried = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{5, 1}, 0, 1, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);
    state.interact();
    state.tick();

    GP_CHECK(state.character_at(carried).state == gp::character_state::idle);
}

GP_TEST(a_rejected_character_can_be_picked_up_again)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    int carried = builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller);
    builder.add(gp::tile_point{5, 1}, 0, 1, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);
    state.interact();
    state.tick();

    // They settled one tile to the player's left, and standing still they are solid again.
    GP_CHECK(state.character_at(carried).position == (gp::tile_point{3, 1}));
    state.turn(gp::direction::left);
    GP_CHECK(!state.try_move(gp::direction::left));
    GP_CHECK_EQ(state.facing_character(), carried);
    GP_CHECK(state.interact() == gp::interact_result::connected);
}

GP_TEST(interacting_with_empty_air_does_nothing)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.interact() == gp::interact_result::nothing);
}

GP_TEST(a_matched_blocker_opens_the_tile_it_held)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    builder.add(gp::tile_point{3, 1}, 0, 0, gp::role::caller, true);
    builder.add(gp::tile_point{5, 1}, 0, 0, gp::role::responder);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.solid(gp::tile_point{3, 1}));

    walk(state, gp::direction::right, 1);
    state.interact();
    state.try_move(gp::direction::right);
    state.try_move(gp::direction::right);
    state.interact();

    GP_CHECK(!state.solid(gp::tile_point{3, 1}));
}

GP_TEST(an_out_of_range_character_index_reads_back_a_blank)
{
    gp_fixture::level_builder builder = make_room(8, 8);
    gp::level_spec spec = builder.spec();
    gp::level_state state(spec);

    GP_CHECK(state.character_at(-1).state == gp::character_state::idle);
    GP_CHECK(state.character_at(99).state == gp::character_state::idle);
}
