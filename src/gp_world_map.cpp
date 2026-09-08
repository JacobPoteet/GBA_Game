/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_world_map.h"

#include "bn_bg_tiles.h"
#include "bn_regular_bg_map_cell_info.h"
#include "bn_size.h"

#include "bn_bg_palette_items_gp_bg_palette.h"
#include "bn_regular_bg_tiles_items_gp_bg_tiles.h"

namespace gp
{

namespace
{

// Frame order in graphics/gp_bg_tiles.bmp, laid out by tools/gp_gen_art.py.
constexpr int void_tile = 0;
constexpr int floor_tile = 1;
constexpr int wall_tile = 2;

/**
 * Tile offsets would shift the indices this map writes by hand, so they are turned off while the
 * background is created. Butano's own dynamic background example does the same.
 */
[[nodiscard]] bn::regular_bg_ptr create_bg(const bn::regular_bg_item& item)
{
    bn::bg_tiles::set_allow_offset(false);
    bn::regular_bg_ptr bg = item.create_bg(0, 0);
    bn::bg_tiles::set_allow_offset(true);
    return bg;
}

} // namespace

world_map::world_map(const collision_map& terrain)
        : _map_item(_cells[0], bn::size(cell_columns, cell_rows)),
          _bg_item(bn::regular_bg_tiles_items::gp_bg_tiles, bn::bg_palette_items::gp_bg_palette, _map_item),
          _bg(create_bg(_bg_item)), _bg_map(_bg.map())
{
    _fill(terrain);
    _bg_map.reload_cells_ref();
}

void world_map::_fill(const collision_map& terrain)
{
    for (int cell_y = 0; cell_y < cell_rows; ++cell_y)
    {
        int tile_y = cell_y / cells_per_tile;

        for (int cell_x = 0; cell_x < cell_columns; ++cell_x)
        {
            int tile_x = cell_x / cells_per_tile;
            int tile_index;

            if (!terrain.contains(tile_point{tile_x, tile_y}))
            {
                // Past the edge of the level, so nothing is drawn and the backdrop shows through.
                tile_index = void_tile;
            }
            else
            {
                tile_index = terrain.blocked(tile_point{tile_x, tile_y}) ? wall_tile : floor_tile;
            }

            bn::regular_bg_map_cell& cell = _cells[_map_item.cell_index(cell_x, cell_y)];
            bn::regular_bg_map_cell_info info(cell);
            info.set_tile_index(tile_index);
            info.set_palette_id(0);
            info.set_horizontal_flip(false);
            info.set_vertical_flip(false);
            cell = info.cell();
        }
    }
}

} // namespace gp
