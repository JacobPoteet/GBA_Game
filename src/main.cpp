/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "bn_core.h"
#include "bn_color.h"
#include "bn_assert.h"
#include "bn_optional.h"
#include "bn_unique_ptr.h"
#include "bn_bg_palettes.h"
#include "bn_sprite_text_generator.h"

#include "common_variable_8x16_sprite_font.h"

#include "gp_ending_scene.h"
#include "gp_interstitial_scene.h"
#include "gp_levels_generated.h"
#include "gp_progress.h"
#include "gp_scene.h"
#include "gp_scene_type.h"
#include "gp_title_scene.h"
#include "gp_world_scene.h"

namespace
{
[[nodiscard]] bn::unique_ptr<gp::scene> create_scene(gp::scene_type type, bn::sprite_text_generator& text_generator,
                                                     gp::progress& run)
{
    switch (type)
    {

    case gp::scene_type::title:
        return bn::unique_ptr<gp::scene>(new gp::title_scene(text_generator, run));

    case gp::scene_type::interstitial:
        return bn::unique_ptr<gp::scene>(new gp::interstitial_scene(text_generator, run));

    case gp::scene_type::world:
        return bn::unique_ptr<gp::scene>(new gp::world_scene(text_generator, run));

    case gp::scene_type::ending:
        return bn::unique_ptr<gp::scene>(new gp::ending_scene(text_generator));
    }

    BN_ERROR("Invalid scene type: ", int(type));
    return bn::unique_ptr<gp::scene>();
}
} // namespace

int main()
{
    bn::core::init();
    bn::bg_palettes::set_transparent_color(bn::color(2, 2, 6));

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    text_generator.set_center_alignment();

    // The only thing that outlives a scene. Scenes are created and destroyed as the player moves
    // between them, so how far along they are cannot live inside one.
    gp::progress run(gp::level_count);

    bn::unique_ptr<gp::scene> scene = create_scene(gp::scene_type::title, text_generator, run);

    while (true)
    {
        if (bn::optional<gp::scene_type> next_scene = scene->update())
        {
            scene = create_scene(*next_scene, text_generator, run);
        }

        bn::core::update();
    }
}
