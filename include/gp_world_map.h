/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_WORLD_MAP_H
#define GP_WORLD_MAP_H

#include "bn_fixed.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_cell.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_ptr.h"

#include "gp_collision_map.h"

namespace gp
{

/**
 * Draws a level's terrain as a background built at runtime.
 *
 * The cells come from the collision map rather than from a painted art layer, so a wall is drawn
 * wherever a wall is solid and the two cannot disagree. The `ground` layer in the Tiled maps is
 * parsed but unused until there is real art to put in it.
 *
 * GBA background cells are 8x8 pixels and the game works in 16 pixel tiles, so one game tile is a
 * 2x2 block of cells. 64x64 cells covers the largest level the collision map allows, and stays
 * inside the size a regular background handles without Butano's big_map path.
 */
class world_map
{

public:
    static constexpr int cells_per_tile = 2;
    static constexpr int cell_columns = collision_map::max_columns * cells_per_tile;
    static constexpr int cell_rows = collision_map::max_rows * cells_per_tile;
    static constexpr int cell_size = 8;

    explicit world_map(const collision_map& terrain);

    [[nodiscard]] bn::regular_bg_ptr& bg()
    {
        return _bg;
    }

    /** Centre of a game tile, in the world coordinates sprites and the camera share. */
    [[nodiscard]] static bn::fixed tile_center(int tile)
    {
        return (tile * cells_per_tile * cell_size) + cell_size - (cell_columns * cell_size / 2);
    }

    /** Half a tile past the outermost tiles, so the camera can be clamped to the level. */
    [[nodiscard]] static bn::fixed edge_before(int tile)
    {
        return tile_center(tile) - cell_size;
    }

    [[nodiscard]] static bn::fixed edge_after(int tile)
    {
        return tile_center(tile) + cell_size;
    }

private:
    alignas(int) bn::regular_bg_map_cell _cells[cell_columns * cell_rows] = {};
    bn::regular_bg_map_item _map_item;
    bn::regular_bg_item _bg_item;
    bn::regular_bg_ptr _bg;
    bn::regular_bg_map_ptr _bg_map;

    void _fill(const collision_map& terrain);
};

} // namespace gp

#endif
