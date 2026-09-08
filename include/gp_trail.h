/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_TRAIL_H
#define GP_TRAIL_H

#include "gp_tile_point.h"

namespace gp
{

/**
 * The tiles the player has stood on, newest first.
 *
 * A follower reads `at(1)` rather than steering toward the player, which is the Pokemon follower
 * model. Three things fall out of it: the follower moves only when the player moves, turning in
 * place leaves the follower where it is, and the follower never clips a wall because it only ever
 * occupies tiles the player already walked.
 */
class trail
{

public:
    static constexpr int capacity = 8;

    /** Clears the trail and fills it with `point`, so a fresh follower stands on the player. */
    void reset(tile_point point);

    /** Records a tile the player moved onto. */
    void push(tile_point point);

    /**
     * `at(0)` is where the player is now, `at(1)` is one step behind. Reads past the recorded
     * history clamp to the oldest tile.
     */
    [[nodiscard]] tile_point at(int steps_behind) const;

    [[nodiscard]] int size() const
    {
        return _size;
    }

private:
    tile_point _points[capacity];
    int _head = 0;
    int _size = 0;
};

} // namespace gp

#endif
