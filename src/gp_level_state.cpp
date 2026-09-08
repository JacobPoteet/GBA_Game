/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_level_state.h"

namespace gp
{

namespace
{
const character invalid_character{};
const level_event invalid_event{};
} // namespace

level_state::level_state(const level_spec& spec)
    : _terrain(spec.columns, spec.rows, spec.collision_bits, spec.collision_byte_count),
      _player_position(spec.player_start)
{
    if (spec.characters)
    {
        int count = spec.character_count > max_characters ? max_characters : spec.character_count;

        for (int index = 0; index < count; ++index)
        {
            const character_spec& source = spec.characters[index];
            character& target = _characters[index];
            target.position = source.position;
            target.sig = source.sig;
            target.state = character_state::idle;
            target.facing = direction::down;
            target.is_blocker = source.is_blocker;
            target.flee_route = source.flee_route;
            target.flee_step = 0;
        }

        _character_count = count;
    }

    if (spec.routes)
    {
        int count = spec.route_count > max_routes ? max_routes : spec.route_count;

        for (int index = 0; index < count; ++index)
        {
            _routes[index] = spec.routes[index];
        }

        _route_count = count;
    }

    _trail.reset(_player_position);
}

const character& level_state::character_at(int index) const
{
    if (index < 0 || index >= _character_count)
    {
        return invalid_character;
    }

    return _characters[index];
}

const level_event& level_state::event_at(int index) const
{
    if (index < 0 || index >= _event_count)
    {
        return invalid_event;
    }

    return _events[index];
}

int level_state::facing_character() const
{
    tile_point target = _player_position + direction_delta(_player_facing);

    for (int index = 0; index < _character_count; ++index)
    {
        const character& value = _characters[index];

        if (value.state != character_state::departed && value.position == target)
        {
            return index;
        }
    }

    return no_character;
}

bool level_state::solid(tile_point point) const
{
    if (_terrain.blocked(point))
    {
        return true;
    }

    for (int index = 0; index < _character_count; ++index)
    {
        const character& value = _characters[index];

        // Everyone standing still is solid. A follower is not, so walking backwards swaps the two
        // of you rather than wedging the player against their own companion, and a runner is not,
        // so a chase cannot pin you in a corridor.
        if (value.state == character_state::idle && value.position == point)
        {
            return true;
        }
    }

    return false;
}

bool level_state::complete() const
{
    for (int index = 0; index < _character_count; ++index)
    {
        if (_characters[index].state != character_state::departed)
        {
            return false;
        }
    }

    return true;
}

bool level_state::turn(direction dir)
{
    if (_player_facing == dir)
    {
        return false;
    }

    _player_facing = dir;
    return true;
}

bool level_state::try_move(direction dir)
{
    if (turn(dir))
    {
        return true;
    }

    tile_point target = _player_position + direction_delta(dir);

    if (solid(target))
    {
        return false;
    }

    _player_position = target;
    _trail.push(target);
    _advance_follower();
    return true;
}

bool level_state::listen()
{
    int target = facing_character();

    if (target == no_character)
    {
        return false;
    }

    _push_event(event_kind::chirp, target, no_character);
    return true;
}

interact_result level_state::interact()
{
    int target = facing_character();

    if (target == no_character)
    {
        return interact_result::nothing;
    }

    if (target == _follower)
    {
        _characters[target].state = character_state::idle;
        _follower = no_character;
        _push_event(event_kind::released, target, no_character);
        return interact_result::released;
    }

    if (_characters[target].state != character_state::idle)
    {
        return interact_result::nothing;
    }

    if (_follower == no_character)
    {
        _characters[target].state = character_state::following;
        _characters[target].position = _trail.at(1);
        _follower = target;
        _push_event(event_kind::connected, target, no_character);
        return interact_result::connected;
    }

    int carried = _follower;

    if (matches(_characters[carried].sig, _characters[target].sig))
    {
        _characters[carried].state = character_state::departed;
        _characters[target].state = character_state::departed;
        _follower = no_character;
        _push_event(event_kind::matched, carried, target);
        _announce_clear_once();
        return interact_result::matched;
    }

    _characters[carried].state = character_state::fleeing;
    _characters[carried].flee_step = 0;
    _follower = no_character;
    _push_event(event_kind::rejected, carried, target);
    return interact_result::rejected;
}

void level_state::tick()
{
    for (int index = 0; index < _character_count; ++index)
    {
        character& value = _characters[index];

        if (value.state != character_state::fleeing)
        {
            continue;
        }

        if (value.flee_route < 0 || value.flee_route >= _route_count)
        {
            value.state = character_state::idle;
            continue;
        }

        const route& path = _routes[value.flee_route];

        if (path.length == 0)
        {
            value.state = character_state::idle;
            continue;
        }

        int next_step = value.flee_step + 1;

        if (next_step >= int(path.length))
        {
            value.position = path.points[path.length - 1];
            value.state = character_state::idle;
            continue;
        }

        tile_point previous = value.position;
        value.flee_step = uint8_t(next_step);
        value.position = path.points[next_step];
        value.facing = direction_towards(previous, value.position);
    }
}

void level_state::_advance_follower()
{
    if (_follower == no_character)
    {
        return;
    }

    character& value = _characters[_follower];
    tile_point previous = value.position;
    value.position = _trail.at(1);

    if (previous != value.position)
    {
        value.facing = direction_towards(previous, value.position);
    }
}

void level_state::_push_event(event_kind kind, int character_index, int other_index)
{
    if (_event_count >= max_events)
    {
        return;
    }

    level_event& target = _events[_event_count];
    target.kind = kind;
    target.character = int8_t(character_index);
    target.other = int8_t(other_index);
    ++_event_count;
}

void level_state::_announce_clear_once()
{
    if (_cleared_announced || !complete())
    {
        return;
    }

    _cleared_announced = true;
    _push_event(event_kind::level_cleared, no_character, no_character);
}

} // namespace gp
