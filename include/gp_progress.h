/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_PROGRESS_H
#define GP_PROGRESS_H

namespace gp
{

/**
 * How far through the game the player is.
 *
 * Scenes are created and destroyed as the player moves between them, so this is owned by `main`
 * and handed to each one by reference. It is the only thing that outlives a scene.
 */
class progress
{

public:
    explicit progress(int level_count) : _level_count(level_count < 0 ? 0 : level_count) {}

    [[nodiscard]] int level() const
    {
        return _level;
    }

    [[nodiscard]] int level_count() const
    {
        return _level_count;
    }

    /** True once the player has walked past the last level. */
    [[nodiscard]] bool finished() const
    {
        return _level >= _level_count;
    }

    [[nodiscard]] bool on_last_level() const
    {
        return _level == _level_count - 1;
    }

    /** Moves to the next level. Past the last one, `finished()` becomes true. */
    void advance()
    {
        if (_level < _level_count)
        {
            ++_level;
        }
    }

    void restart()
    {
        _level = 0;
    }

private:
    int _level = 0;
    int _level_count = 0;
};

} // namespace gp

#endif
