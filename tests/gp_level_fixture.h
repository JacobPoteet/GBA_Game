/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_LEVEL_FIXTURE_H
#define GP_LEVEL_FIXTURE_H

#include <cstdint>
#include <initializer_list>
#include <vector>

#include "gp_level_data.h"

namespace gp_fixture
{

/**
 * Builds a level_spec in memory so tests do not need the map importer.
 *
 * The spec points into this object, so keep the builder alive for as long as the spec is used.
 */
class level_builder
{

public:
    level_builder(int columns, int rows) : _columns(columns), _rows(rows)
    {
        _bits.assign(size_t((columns * rows + 7) / 8), 0);
    }

    void wall(int x, int y)
    {
        int bit = (y * _columns) + x;
        _bits[size_t(bit / 8)] = uint8_t(_bits[size_t(bit / 8)] | uint8_t(1 << (bit % 8)));
    }

    void start(int x, int y)
    {
        _start = gp::tile_point{x, y};
    }

    int add(gp::tile_point position, int clan, int family, gp::role part, bool is_blocker = false,
            int flee_route = gp::no_route)
    {
        gp::character_spec value;
        value.position = position;
        value.sig.clan = uint8_t(clan);
        value.sig.family = uint8_t(family);
        value.sig.part = part;
        value.is_blocker = is_blocker;
        value.flee_route = int8_t(flee_route);
        _characters.push_back(value);
        return int(_characters.size()) - 1;
    }

    int add_route(std::initializer_list<gp::tile_point> points)
    {
        gp::route value;
        uint8_t length = 0;

        for (gp::tile_point point : points)
        {
            if (length >= gp::max_route_points)
            {
                break;
            }

            value.points[length] = point;
            ++length;
        }

        value.length = length;
        _routes.push_back(value);
        return int(_routes.size()) - 1;
    }

    [[nodiscard]] gp::level_spec spec() const
    {
        gp::level_spec value;
        value.name = "fixture";
        value.columns = _columns;
        value.rows = _rows;
        value.collision_bits = _bits.data();
        value.collision_byte_count = int(_bits.size());
        value.player_start = _start;
        value.characters = _characters.empty() ? nullptr : _characters.data();
        value.character_count = int(_characters.size());
        value.routes = _routes.empty() ? nullptr : _routes.data();
        value.route_count = int(_routes.size());
        return value;
    }

private:
    std::vector<uint8_t> _bits;
    std::vector<gp::character_spec> _characters;
    std::vector<gp::route> _routes;
    int _columns;
    int _rows;
    gp::tile_point _start{1, 1};
};

} // namespace gp_fixture

#endif
