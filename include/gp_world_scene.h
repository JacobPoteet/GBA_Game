/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_WORLD_SCENE_H
#define GP_WORLD_SCENE_H

#include "bn_camera_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gp_chirp_player.h"
#include "gp_level_data.h"
#include "gp_level_state.h"
#include "gp_scene.h"
#include "gp_world_map.h"

namespace bn
{
class sprite_text_generator;
}

namespace gp
{

class progress;

/**
 * Everything the player can see and hear.
 *
 * It owns the background, the camera and every sprite, and it decides nothing: it feeds input to
 * `gp::level_state` and drains the events that come back. Every rule lives on the other side of
 * that line, which is what keeps the rules testable. See the wiki's The Pure Split note.
 */
class world_scene : public scene
{

public:
    world_scene(bn::sprite_text_generator& text_generator, progress& run);

    [[nodiscard]] bn::optional<scene_type> update() override;

private:
    /** Pixels a sprite covers per frame. One tile is 16 pixels. */
    static constexpr int walk_step = 2;
    static constexpr int run_step = 4;

    /** Frames a runner spends on each tile of its flee route. */
    static constexpr int flee_frames = 4;

    /** Longer than this and B was a run, not a request to listen. */
    static constexpr int listen_frames = 12;

    bn::sprite_text_generator& _text_generator;
    progress& _progress;
    level_state _state;
    world_map _map;
    chirp_player _chirps;
    bn::camera_ptr _camera;
    bn::sprite_ptr _player_sprite;
    bn::vector<bn::sprite_ptr, max_characters> _character_sprites;
    bn::vector<bn::sprite_ptr, 24> _hud_sprites;

    int _b_frames = 0;
    int _flee_countdown = flee_frames;
    int _hud_follower = no_character;
    bool _hud_drawn = false;

    [[nodiscard]] bool _player_arrived() const;
    void _handle_input();
    void _drain_events();
    void _advance_sprites();
    void _update_camera();
    void _update_hud();
};

} // namespace gp

#endif
