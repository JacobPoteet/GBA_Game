/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_interstitial_scene.h"

#include "bn_keypad.h"
#include "bn_sprite_text_generator.h"

#include "gp_progress.h"
#include "gp_story.h"

namespace gp
{

namespace
{

/** Frames before A does anything, so the button that cleared a level cannot skip the card. */
constexpr int settle_frames = 20;

} // namespace

interstitial_scene::interstitial_scene(bn::sprite_text_generator& text_generator, const progress& run)
{
    story_card card = story_before(run.level());
    text_generator.generate(0, -12, card.first, _text_sprites);
    text_generator.generate(0, 8, card.second, _text_sprites);
}

bn::optional<scene_type> interstitial_scene::update()
{
    if (_settle < settle_frames)
    {
        ++_settle;
        return bn::nullopt;
    }

    if (bn::keypad::a_pressed() || bn::keypad::start_pressed())
    {
        return scene_type::world;
    }

    return bn::nullopt;
}

} // namespace gp
