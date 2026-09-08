/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_ENDING_SCENE_H
#define GP_ENDING_SCENE_H

#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gp_scene.h"

namespace bn
{
class sprite_text_generator;
}

namespace gp
{

/** The last thing the game says, once the player has found their own. */
class ending_scene : public scene
{

public:
    explicit ending_scene(bn::sprite_text_generator& text_generator);

    [[nodiscard]] bn::optional<scene_type> update() override;

private:
    bn::vector<bn::sprite_ptr, 32> _text_sprites;
    int _settle = 0;
};

} // namespace gp

#endif
