/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_game_scene.h"

#include "bn_keypad.h"
#include "bn_format.h"
#include "bn_sprite_text_generator.h"

#include "bn_sprite_items_gp_tile.h"
#include "bn_sprite_items_gp_cursor.h"

namespace gp
{

namespace
{
constexpr int tile_size = 16;
constexpr int board_left = -((board::columns - 1) * tile_size) / 2;
constexpr int board_top = (-((board::rows - 1) * tile_size) / 2) + 8;

[[nodiscard]] constexpr int cell_x(int column)
{
    return board_left + (column * tile_size);
}

[[nodiscard]] constexpr int cell_y(int row)
{
    return board_top + (row * tile_size);
}
} // namespace

game_scene::game_scene(bn::sprite_text_generator& text_generator)
        : _text_generator(text_generator),
          _cursor_sprite(bn::sprite_items::gp_cursor.create_sprite(cell_x(_cursor.column()), cell_y(_cursor.row()))),
          _seed(0x2545F491)
{
    // Drawn in front of the tiles.
    _cursor_sprite.set_z_order(-1);

    _board.fill(_seed);
    _rebuild_tile_sprites();
    _update_text();
}

bn::optional<scene_type> game_scene::update()
{
    bool cursor_moved = false;
    bool board_changed = false;

    if (bn::keypad::left_pressed())
    {
        cursor_moved = _cursor.move(-1, 0) || cursor_moved;
    }

    if (bn::keypad::right_pressed())
    {
        cursor_moved = _cursor.move(1, 0) || cursor_moved;
    }

    if (bn::keypad::up_pressed())
    {
        cursor_moved = _cursor.move(0, -1) || cursor_moved;
    }

    if (bn::keypad::down_pressed())
    {
        cursor_moved = _cursor.move(0, 1) || cursor_moved;
    }

    if (bn::keypad::a_pressed())
    {
        int column = _cursor.column();
        int row = _cursor.row();
        _board.set_tile(column, row, next_tile_kind(_board.tile(column, row)));
        board_changed = true;
    }

    if (bn::keypad::start_pressed())
    {
        // Golden ratio increment, so successive shuffles do not repeat.
        _seed += 0x9E3779B9;
        _board.fill(_seed);
        board_changed = true;
    }

    if (board_changed)
    {
        _rebuild_tile_sprites();
    }

    if (cursor_moved)
    {
        _update_cursor_sprite();
    }

    if (cursor_moved || board_changed)
    {
        _update_text();
    }

    if (bn::keypad::b_pressed())
    {
        return scene_type::title;
    }

    return bn::nullopt;
}

void game_scene::_rebuild_tile_sprites()
{
    _tile_sprites.clear();

    for (int row = 0; row < board::rows; ++row)
    {
        for (int column = 0; column < board::columns; ++column)
        {
            tile_kind kind = _board.tile(column, row);

            if (!is_empty(kind))
            {
                _tile_sprites.push_back(bn::sprite_items::gp_tile.create_sprite(cell_x(column), cell_y(row),
                                                                                tile_kind_graphics_index(kind)));
            }
        }
    }
}

void game_scene::_update_cursor_sprite()
{
    _cursor_sprite.set_position(cell_x(_cursor.column()), cell_y(_cursor.row()));
}

void game_scene::_update_text()
{
    _text_sprites.clear();
    _text_generator.generate(0, -68, bn::format<32>("CELL {},{}", _cursor.column(), _cursor.row()), _text_sprites);
    _text_generator.generate(0, 66, "A CYCLE   START SHUFFLE   B BACK", _text_sprites);
}

} // namespace gp
