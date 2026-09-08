/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_story.h"

namespace gp
{

namespace
{

/*
 * Draft text. The arc is the same shape as the levels: willing, then tired, then quiet, then found.
 * Keep each line under about 28 characters or the 8x16 font runs off a 240 pixel screen.
 */
constexpr story_card cards[] = {
        {"Everyone here is looking.", "So am I."},       {"I am getting good at this.", "It has not helped me yet."},
        {"They sound alike at first.", "They are not."}, {"People stand in doorways.", "They do not know it."},
        {"One more room.", "I have stopped expecting."},
};

constexpr int card_count = int(sizeof(cards) / sizeof(cards[0]));

constexpr story_card ending{"Oh.", "There you are."};

} // namespace

int story_card_count()
{
    return card_count;
}

story_card story_before(int level_index)
{
    if (level_index < 0 || level_index >= card_count)
    {
        return cards[0];
    }

    return cards[level_index];
}

story_card story_ending()
{
    return ending;
}

} // namespace gp
