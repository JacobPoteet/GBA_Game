/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_title_scene.h"

#include "bn_keypad.h"
#include "bn_sprite_text_generator.h"

namespace gp
{

title_scene::title_scene(bn::sprite_text_generator& text_generator)
{
    text_generator.generate(0, -32, "CHIRP", _text_sprites);
    text_generator.generate(0, 0, "PRESS START", _text_sprites);
    text_generator.generate(0, 56, "LOOKING FOR A MATCH", _text_sprites);
}

bn::optional<scene_type> title_scene::update()
{
    if (bn::keypad::start_pressed() || bn::keypad::a_pressed())
    {
        return scene_type::world;
    }

    return bn::nullopt;
}

} // namespace gp
