/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_world_scene.h"

#include "bn_format.h"
#include "bn_keypad.h"
#include "bn_sprite_text_generator.h"

#include "bn_sprite_items_gp_character.h"
#include "bn_sprite_items_gp_player.h"

#include "gp_levels_generated.h"
#include "gp_progress.h"

namespace gp
{

namespace
{

constexpr int screen_half_width = 120;
constexpr int screen_half_height = 80;
constexpr int hud_top_y = -72;
constexpr int hud_bottom_y = 66;

[[nodiscard]] bn::fixed approach(bn::fixed current, bn::fixed target, bn::fixed step)
{
    if (current < target)
    {
        current += step;
        return current > target ? target : current;
    }

    if (current > target)
    {
        current -= step;
        return current < target ? target : current;
    }

    return current;
}

void step_sprite(bn::sprite_ptr& sprite, tile_point tile, bn::fixed step)
{
    sprite.set_position(approach(sprite.x(), world_map::tile_center(tile.x), step),
                        approach(sprite.y(), world_map::tile_center(tile.y), step));
}

[[nodiscard]] bn::fixed clamp_camera(bn::fixed value, bn::fixed low, bn::fixed high, int half_extent)
{
    if (high - low <= half_extent * 2)
    {
        return (low + high) / 2;
    }

    if (value < low + half_extent)
    {
        return low + half_extent;
    }

    if (value > high - half_extent)
    {
        return high - half_extent;
    }

    return value;
}

[[nodiscard]] const level_spec& level_at(int index)
{
    int safe = index;

    if (safe < 0 || safe >= level_count)
    {
        safe = 0;
    }

    return levels[safe];
}

} // namespace

world_scene::world_scene(bn::sprite_text_generator& text_generator, progress& run)
        : _text_generator(text_generator), _progress(run), _state(level_at(run.level())), _map(_state.terrain()),
          _camera(bn::camera_ptr::create(world_map::tile_center(_state.player_position().x),
                                         world_map::tile_center(_state.player_position().y))),
          _player_sprite(bn::sprite_items::gp_player.create_sprite(world_map::tile_center(_state.player_position().x),
                                                                   world_map::tile_center(_state.player_position().y),
                                                                   int(_state.player_facing())))
{
    _map.bg().set_camera(_camera);
    _player_sprite.set_camera(_camera);

    // Drawn in front of everyone, so a follower standing behind never hides the player.
    _player_sprite.set_z_order(-1);

    for (int index = 0; index < _state.character_count(); ++index)
    {
        const character& value = _state.character_at(index);
        bn::sprite_ptr sprite = bn::sprite_items::gp_character.create_sprite(world_map::tile_center(value.position.x),
                                                                             world_map::tile_center(value.position.y),
                                                                             signature_graphics_index(value.sig));
        sprite.set_camera(_camera);
        _character_sprites.push_back(sprite);
    }

    _update_camera();
    _update_hud();
}

bool world_scene::_player_arrived() const
{
    return _player_sprite.x() == world_map::tile_center(_state.player_position().x) &&
           _player_sprite.y() == world_map::tile_center(_state.player_position().y);
}

bn::optional<scene_type> world_scene::update()
{
    _handle_input();
    _drain_events();
    _advance_sprites();
    _update_camera();
    _update_hud();
    _chirps.update();

    if (_state.complete() && bn::keypad::a_pressed())
    {
        if (_progress.on_last_level())
        {
            return scene_type::ending;
        }

        _progress.advance();
        return scene_type::interstitial;
    }

    return bn::nullopt;
}

void world_scene::_handle_input()
{
    if (bn::keypad::b_held())
    {
        ++_b_frames;
    }

    // A short press asks somebody to chirp. Holding B is a run, and asks for nothing.
    if (bn::keypad::b_released())
    {
        if (_b_frames <= listen_frames)
        {
            _state.listen();
        }

        _b_frames = 0;
    }

    if (bn::keypad::a_pressed() && !_state.complete())
    {
        _state.interact();
    }

    if (!_player_arrived())
    {
        return;
    }

    if (bn::keypad::left_held())
    {
        _state.try_move(direction::left);
    }
    else if (bn::keypad::right_held())
    {
        _state.try_move(direction::right);
    }
    else if (bn::keypad::up_held())
    {
        _state.try_move(direction::up);
    }
    else if (bn::keypad::down_held())
    {
        _state.try_move(direction::down);
    }

    _player_sprite.set_item(bn::sprite_items::gp_player, int(_state.player_facing()));
}

void world_scene::_drain_events()
{
    for (int index = 0; index < _state.event_count(); ++index)
    {
        const level_event& value = _state.event_at(index);

        switch (value.kind)
        {

        case event_kind::chirp:
            _chirps.play(_state.character_at(value.character).sig);
            break;

        case event_kind::connected:
        case event_kind::released:
            _hud_drawn = false;
            break;

        case event_kind::matched:
            // Both halves of the pair walk off together, so their sprites go with them.
            if (value.character >= 0 && value.character < int(_character_sprites.size()))
            {
                _character_sprites[value.character].set_visible(false);
            }

            if (value.other >= 0 && value.other < int(_character_sprites.size()))
            {
                _character_sprites[value.other].set_visible(false);
            }

            _hud_drawn = false;
            break;

        case event_kind::rejected:
            _hud_drawn = false;
            break;

        case event_kind::player_matched:
            if (value.character >= 0 && value.character < _character_sprites.size())
            {
                _character_sprites[value.character].set_visible(false);
            }

            _hud_drawn = false;
            break;

        case event_kind::level_cleared:
            _hud_drawn = false;
            break;
        }
    }

    _state.clear_events();
}

void world_scene::_advance_sprites()
{
    bn::fixed player_step = bn::keypad::b_held() ? run_step : walk_step;
    step_sprite(_player_sprite, _state.player_position(), player_step);

    for (int index = 0; index < _state.character_count(); ++index)
    {
        step_sprite(_character_sprites[index], _state.character_at(index).position, run_step);
    }

    if (_flee_countdown > 0)
    {
        --_flee_countdown;
    }
    else
    {
        _state.tick();
        _flee_countdown = flee_frames;
    }
}

void world_scene::_update_camera()
{
    const collision_map& terrain = _state.terrain();
    bn::fixed left = world_map::edge_before(0);
    bn::fixed right = world_map::edge_after(terrain.columns() - 1);
    bn::fixed top = world_map::edge_before(0);
    bn::fixed bottom = world_map::edge_after(terrain.rows() - 1);

    _camera.set_position(clamp_camera(_player_sprite.x(), left, right, screen_half_width),
                         clamp_camera(_player_sprite.y(), top, bottom, screen_half_height));
}

void world_scene::_update_hud()
{
    if (_hud_drawn && _hud_follower == _state.follower())
    {
        return;
    }

    _hud_follower = _state.follower();
    _hud_drawn = true;
    _hud_sprites.clear();

    if (_state.complete())
    {
        _text_generator.generate(0, hud_top_y, "EVERYONE FOUND SOMEONE", _hud_sprites);
        _text_generator.generate(0, hud_bottom_y, "A: MOVE ON", _hud_sprites);
        return;
    }

    if (_hud_follower == no_character)
    {
        _text_generator.generate(0, hud_top_y, "ALONE", _hud_sprites);
    }
    else
    {
        const signature& sig = _state.character_at(_hud_follower).sig;
        _text_generator.generate(
                0, hud_top_y,
                bn::format<24>("WITH CLAN {} {}", int(sig.clan), sig.part == role::caller ? "CALLER" : "ANSWER"),
                _hud_sprites);
    }

    _text_generator.generate(0, hud_bottom_y, "A: CONNECT   B: LISTEN", _hud_sprites);
}

} // namespace gp
