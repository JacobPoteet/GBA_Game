/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_INTERSTITIAL_SCENE_H
#define GP_INTERSTITIAL_SCENE_H

#include "bn_sprite_ptr.h"
#include "bn_vector.h"

#include "gp_scene.h"

namespace bn
{
class sprite_text_generator;
}

namespace gp
{

class progress;

/**
 * Two lines on a plain field, between one level and the next.
 *
 * It knows nothing about what it says. The lines come from `gp::story_before`, which is data in the
 * pure layer so a test can check the story covers every level.
 */
class interstitial_scene : public scene
{

public:
    interstitial_scene(bn::sprite_text_generator& text_generator, const progress& run);

    [[nodiscard]] bn::optional<scene_type> update() override;

private:
    bn::vector<bn::sprite_ptr, 32> _text_sprites;
    int _settle = 0;
};

} // namespace gp

#endif
