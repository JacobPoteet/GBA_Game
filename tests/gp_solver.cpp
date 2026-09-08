/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_solver.h"

#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "gp_collision_map.h"
#include "gp_direction.h"

namespace gp
{

namespace
{

bool blocked_for(const level_spec& spec, const collision_map& terrain, uint32_t departed, tile_point point)
{
    if (terrain.blocked(point))
    {
        return true;
    }

    for (int index = 0; index < spec.character_count; ++index)
    {
        if (departed & (uint32_t(1) << index))
        {
            continue;
        }

        if (spec.characters[index].position == point)
        {
            return true;
        }
    }

    return false;
}

std::vector<char> flood(const level_spec& spec, const collision_map& terrain, uint32_t departed)
{
    int columns = terrain.columns();
    int rows = terrain.rows();
    std::vector<char> seen(size_t(columns < 0 ? 0 : columns) * size_t(rows < 0 ? 0 : rows), 0);

    if (columns <= 0 || rows <= 0)
    {
        return seen;
    }

    tile_point start = spec.player_start;

    if (blocked_for(spec, terrain, departed, start))
    {
        return seen;
    }

    std::vector<tile_point> pending;
    pending.push_back(start);
    seen[size_t(start.y * columns + start.x)] = 1;

    while (!pending.empty())
    {
        tile_point current = pending.back();
        pending.pop_back();

        for (int step = 0; step < direction_count; ++step)
        {
            tile_point next = current + direction_delta(direction(step));

            if (!terrain.contains(next))
            {
                continue;
            }

            size_t offset = size_t(next.y * columns + next.x);

            if (seen[offset] || blocked_for(spec, terrain, departed, next))
            {
                continue;
            }

            seen[offset] = 1;
            pending.push_back(next);
        }
    }

    return seen;
}

/** True when the player can stand on a tile beside this character and face them. */
bool approachable(const collision_map& terrain, const std::vector<char>& reachable, tile_point position)
{
    int columns = terrain.columns();

    for (int step = 0; step < direction_count; ++step)
    {
        tile_point neighbour = position + direction_delta(direction(step));

        if (!terrain.contains(neighbour))
        {
            continue;
        }

        if (reachable[size_t(neighbour.y * columns + neighbour.x)])
        {
            return true;
        }
    }

    return false;
}

/** Everyone the player is expected to pair off, which is everyone but their own match. */
uint32_t pairable_mask(const level_spec& spec)
{
    uint32_t mask = 0;

    for (int index = 0; index < spec.character_count; ++index)
    {
        if (!matches(player_signature, spec.characters[index].sig))
        {
            mask |= uint32_t(1) << index;
        }
    }

    return mask;
}

bool search(const level_spec& spec, const collision_map& terrain, uint32_t departed, uint32_t pairable,
            std::unordered_set<uint32_t>& failed, std::vector<solve_step>& steps)
{
    if (departed == pairable)
    {
        return true;
    }

    if (failed.count(departed))
    {
        return false;
    }

    std::vector<char> reachable = flood(spec, terrain, departed);

    for (int first = 0; first < spec.character_count; ++first)
    {
        uint32_t first_bit = uint32_t(1) << first;

        if ((departed & first_bit) || !(pairable & first_bit))
        {
            continue;
        }

        if (!approachable(terrain, reachable, spec.characters[first].position))
        {
            continue;
        }

        // While the player carries `first`, the tile `first` was standing on is open, so a
        // character walled in behind them comes into reach.
        std::vector<char> carrying = flood(spec, terrain, departed | first_bit);

        for (int second = first + 1; second < spec.character_count; ++second)
        {
            uint32_t second_bit = uint32_t(1) << second;

            if ((departed & second_bit) || !(pairable & second_bit))
            {
                continue;
            }

            if (!matches(spec.characters[first].sig, spec.characters[second].sig))
            {
                continue;
            }

            if (!approachable(terrain, carrying, spec.characters[second].position))
            {
                continue;
            }

            if (search(spec, terrain, departed | first_bit | second_bit, pairable, failed, steps))
            {
                steps.push_back(solve_step{first, second});
                return true;
            }
        }
    }

    failed.insert(departed);
    return false;
}

} // namespace

solve_result solve(const level_spec& spec)
{
    solve_result result;

    if (spec.character_count <= 0)
    {
        result.solvable = true;
        return result;
    }

    if (!spec.characters || spec.character_count > 31)
    {
        return result;
    }

    uint32_t pairable = pairable_mask(spec);
    int pairable_count = 0;

    for (int index = 0; index < spec.character_count; ++index)
    {
        if (pairable & (uint32_t(1) << index))
        {
            ++pairable_count;
        }
    }

    // Everyone but the player's own match has to pair off, so an odd count can never work.
    if (pairable_count % 2 != 0)
    {
        return result;
    }

    collision_map terrain(spec.columns, spec.rows, spec.collision_bits, spec.collision_byte_count);
    std::unordered_set<uint32_t> failed;

    if (!search(spec, terrain, 0, pairable, failed, result.steps))
    {
        return result;
    }

    std::reverse(result.steps.begin(), result.steps.end());

    // The player's own match is resolved last, and only once everybody else has gone. They have to
    // be standing somewhere the player can still reach at that point.
    std::vector<char> reachable = flood(spec, terrain, pairable);

    for (int index = 0; index < spec.character_count; ++index)
    {
        if (pairable & (uint32_t(1) << index))
        {
            continue;
        }

        result.player_match = index;

        if (!approachable(terrain, reachable, spec.characters[index].position))
        {
            result.steps.clear();
            return result;
        }
    }

    result.solvable = true;
    return result;
}

} // namespace gp
