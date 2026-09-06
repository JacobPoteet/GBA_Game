/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_GAME_SCENE_H
#define GP_GAME_SCENE_H

#include "bn_vector.h"
#include "bn_sprite_ptr.h"

#include "gp_scene.h"
#include "gp_board.h"
#include "gp_cursor.h"

namespace bn
{
class sprite_text_generator;
}

namespace gp
{

/**
 * Placeholder gameplay screen.
 *
 * There are no puzzle rules yet: this exists to prove the board, the cursor, the sprite mapping
 * and the input plumbing all work end to end on hardware. Rules go on top of gp::board.
 */
class game_scene : public scene
{

public:
    explicit game_scene(bn::sprite_text_generator& text_generator);

    [[nodiscard]] bn::optional<scene_type> update() override;

private:
    bn::sprite_text_generator& _text_generator;
    board _board;
    cursor _cursor;
    bn::vector<bn::sprite_ptr, board::cell_count> _tile_sprites;
    bn::vector<bn::sprite_ptr, 32> _text_sprites;
    bn::sprite_ptr _cursor_sprite;
    uint32_t _seed;

    void _rebuild_tile_sprites();

    void _update_cursor_sprite();

    void _update_text();
};

} // namespace gp

#endif
