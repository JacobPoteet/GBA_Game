/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_LEVEL_STATE_H
#define GP_LEVEL_STATE_H

#include <cstdint>

#include "gp_collision_map.h"
#include "gp_direction.h"
#include "gp_level_data.h"
#include "gp_signature.h"
#include "gp_tile_point.h"
#include "gp_trail.h"

namespace gp
{

enum class character_state : uint8_t
{
    /** Standing in the world, available to connect with. */
    idle = 0,

    /** Attached to the player, walking the trail. */
    following,

    /** Rejected, walking away along an authored route. */
    fleeing,

    /** Matched and gone. Whatever tile they held is now open. */
    departed
};

struct character
{
    tile_point position;
    signature sig;
    character_state state = character_state::idle;
    direction facing = direction::down;
    bool is_blocker = false;
    int8_t flee_route = int8_t(no_route);
    uint8_t flee_step = 0;
};

enum class event_kind : uint8_t
{
    chirp = 0,
    connected,
    matched,
    rejected,
    released,

    /** The player found their own. Only ever fires once, in the last level. */
    player_matched,

    level_cleared
};

/**
 * Something the presentation layer has to show or play. The simulation never draws and never makes
 * a sound; it queues one of these and lets the scene drain the queue each frame. That indirection
 * is what keeps the whole simulation host-testable.
 */
struct level_event
{
    event_kind kind = event_kind::chirp;
    int8_t character = int8_t(no_character);
    int8_t other = int8_t(no_character);
};

enum class interact_result : uint8_t
{
    nothing = 0,
    connected,
    matched,
    rejected,
    released,

    /** The player's own match is here, but everyone else still needs sorting. */
    waiting,

    player_matched
};

/**
 * The whole game, with no picture and no sound.
 */
class level_state
{

public:
    static constexpr int max_events = 8;

    explicit level_state(const level_spec& spec);

    [[nodiscard]] tile_point player_position() const
    {
        return _player_position;
    }

    [[nodiscard]] direction player_facing() const
    {
        return _player_facing;
    }

    [[nodiscard]] const collision_map& terrain() const
    {
        return _terrain;
    }

    [[nodiscard]] int character_count() const
    {
        return _character_count;
    }

    [[nodiscard]] const character& character_at(int index) const;

    /** The character walking behind the player, or `no_character`. */
    [[nodiscard]] int follower() const
    {
        return _follower;
    }

    /** Who stands on the tile the player faces, or `no_character`. */
    [[nodiscard]] int facing_character() const;

    /**
     * The character this level holds for the player, or `no_character`.
     *
     * Only the last level has one. They cannot be carried and cannot be introduced to anybody, so
     * the only way to resolve them is to be the last thing you do.
     */
    [[nodiscard]] int player_match() const;

    /** Terrain, plus any idle blocker standing in the way. */
    [[nodiscard]] bool solid(tile_point point) const;

    [[nodiscard]] bool complete() const;

    /** Points the player without moving them. Returns true when the facing changed. */
    bool turn(direction dir);

    /**
     * Turns to face `dir` when the player is looking elsewhere, and steps that way when they are
     * already facing it. Returns true when the player turned or moved.
     *
     * Turning and walking share one button so that facing your own follower costs a tap rather than
     * a lap around them.
     */
    bool try_move(direction dir);

    /** Queues a chirp from whoever the player faces. Returns false when nobody is there. */
    bool listen();

    /** Connects, attempts a match, or lets the current follower go. */
    interact_result interact();

    /** Advances every fleeing character one tile along its route. */
    void tick();

    [[nodiscard]] int event_count() const
    {
        return _event_count;
    }

    [[nodiscard]] const level_event& event_at(int index) const;

    void clear_events()
    {
        _event_count = 0;
    }

private:
    collision_map _terrain;
    character _characters[max_characters];
    route _routes[max_routes];
    level_event _events[max_events];
    trail _trail;

    tile_point _player_position;
    direction _player_facing = direction::down;
    int _character_count = 0;
    int _route_count = 0;
    int _follower = no_character;
    int _event_count = 0;
    bool _cleared_announced = false;

    [[nodiscard]] bool _everyone_else_departed(int except) const;
    void _push_event(event_kind kind, int character_index, int other_index);
    void _advance_follower();
    void _announce_clear_once();
};

} // namespace gp

#endif
