/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_SCENE_H
#define GP_SCENE_H

#include "bn_optional.h"

#include "gp_scene_type.h"

namespace gp
{

/**
 * One screen of the game.
 *
 * A scene owns its sprites for its whole lifetime, so swapping scenes is just destroying one
 * object and constructing the next. update() returns the scene to switch to, or nothing to stay.
 */
class scene
{

public:
    virtual ~scene() = default;

    [[nodiscard]] virtual bn::optional<scene_type> update() = 0;

protected:
    scene() = default;
};

} // namespace gp

#endif
