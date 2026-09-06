/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_TITLE_SCENE_H
#define GP_TITLE_SCENE_H

#include "bn_vector.h"
#include "bn_sprite_ptr.h"

#include "gp_scene.h"

namespace bn
{
class sprite_text_generator;
}

namespace gp
{

class title_scene : public scene
{

public:
    explicit title_scene(bn::sprite_text_generator& text_generator);

    [[nodiscard]] bn::optional<scene_type> update() override;

private:
    bn::vector<bn::sprite_ptr, 32> _text_sprites;
};

} // namespace gp

#endif
