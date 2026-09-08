/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_title_scene.h"

#include "bn_sprite_text_generator.h"

namespace gp
{

title_scene::title_scene(bn::sprite_text_generator& text_generator)
{
    text_generator.generate(0, -32, "CHIRP", _text_sprites);
    text_generator.generate(0, 0, "LOOKING FOR A MATCH", _text_sprites);
    text_generator.generate(0, 56, "SIMULATION ONLY", _text_sprites);
}

bn::optional<scene_type> title_scene::update()
{
    // The world scene arrives in phase 3. Until then the simulation runs in the host tests and the
    // ROM has nowhere to go.
    return bn::nullopt;
}

} // namespace gp
