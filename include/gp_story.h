/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_STORY_H
#define GP_STORY_H

namespace gp
{

/**
 * Two lines on a plain field, shown between levels.
 *
 * Kept in the pure layer because it is data, not drawing, which lets a test check that the story
 * covers every level. The scene that shows a card knows nothing about what it says.
 */
struct story_card
{
    const char* first = "";
    const char* second = "";
};

/** How many cards exist. There has to be at least one per level. */
[[nodiscard]] int story_card_count();

/** The card shown before a level starts. Out of range reads as the first card. */
[[nodiscard]] story_card story_before(int level_index);

/** The last thing the game says. */
[[nodiscard]] story_card story_ending();

} // namespace gp

#endif
