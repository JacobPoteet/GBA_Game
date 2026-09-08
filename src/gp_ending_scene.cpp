/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_ending_scene.h"

#include "bn_keypad.h"
#include "bn_sprite_text_generator.h"

#include "gp_story.h"

namespace gp
{

namespace
{

/** Long enough that the last line is read rather than skipped past. */
constexpr int settle_frames = 90;

} // namespace

ending_scene::ending_scene(bn::sprite_text_generator& text_generator)
{
    story_card card = story_ending();
    text_generator.generate(0, -12, card.first, _text_sprites);
    text_generator.generate(0, 8, card.second, _text_sprites);
}

bn::optional<scene_type> ending_scene::update()
{
    if (_settle < settle_frames)
    {
        ++_settle;
        return bn::nullopt;
    }

    if (bn::keypad::a_pressed() || bn::keypad::start_pressed())
    {
        return scene_type::title;
    }

    return bn::nullopt;
}

} // namespace gp
