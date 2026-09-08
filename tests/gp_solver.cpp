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

bool search(const level_spec& spec, const collision_map& terrain, uint32_t departed, uint32_t all,
            std::unordered_set<uint32_t>& failed, std::vector<solve_step>& steps)
{
    if (departed == all)
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
        if (departed & (uint32_t(1) << first))
        {
            continue;
        }

        if (!approachable(terrain, reachable, spec.characters[first].position))
        {
            continue;
        }

        // While the player carries `first`, the tile `first` was standing on is open, so a
        // character walled in behind them comes into reach.
        std::vector<char> carrying = flood(spec, terrain, departed | (uint32_t(1) << first));

        for (int second = first + 1; second < spec.character_count; ++second)
        {
            if (departed & (uint32_t(1) << second))
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

            uint32_t next = departed | (uint32_t(1) << first) | (uint32_t(1) << second);

            if (search(spec, terrain, next, all, failed, steps))
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

    if (!spec.characters || spec.character_count % 2 != 0)
    {
        return result;
    }

    collision_map terrain(spec.columns, spec.rows, spec.collision_bits, spec.collision_byte_count);
    uint32_t all = spec.character_count >= 32 ? 0xFFFFFFFFu : (uint32_t(1) << spec.character_count) - 1;
    std::unordered_set<uint32_t> failed;

    result.solvable = search(spec, terrain, 0, all, failed, result.steps);
    std::reverse(result.steps.begin(), result.steps.end());
    return result;
}

} // namespace gp
