/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_levels_generated.h"
#include "gp_progress.h"
#include "gp_story.h"

GP_TEST(a_new_game_starts_on_the_first_level)
{
    gp::progress run(3);
    GP_CHECK_EQ(run.level(), 0);
    GP_CHECK_EQ(run.level_count(), 3);
    GP_CHECK(!run.finished());
    GP_CHECK(!run.on_last_level());
}

GP_TEST(advancing_walks_through_every_level_and_then_stops)
{
    gp::progress run(3);
    run.advance();
    GP_CHECK_EQ(run.level(), 1);

    run.advance();
    GP_CHECK_EQ(run.level(), 2);
    GP_CHECK(run.on_last_level());
    GP_CHECK(!run.finished());

    run.advance();
    GP_CHECK(run.finished());

    // Past the end it stays put rather than running away.
    run.advance();
    GP_CHECK_EQ(run.level(), 3);
}

GP_TEST(restarting_returns_to_the_first_level)
{
    gp::progress run(3);
    run.advance();
    run.advance();
    run.restart();
    GP_CHECK_EQ(run.level(), 0);
    GP_CHECK(!run.finished());
}

GP_TEST(a_game_with_no_levels_is_finished_before_it_starts)
{
    gp::progress run(0);
    GP_CHECK(run.finished());
}

GP_TEST(a_negative_level_count_is_treated_as_none)
{
    gp::progress run(-4);
    GP_CHECK_EQ(run.level_count(), 0);
    GP_CHECK(run.finished());
}

GP_TEST(the_story_has_a_card_for_every_level)
{
    GP_CHECK(gp::story_card_count() >= gp::level_count);
}

GP_TEST(every_card_says_something)
{
    for (int index = 0; index < gp::story_card_count(); ++index)
    {
        gp::story_card card = gp::story_before(index);
        GP_CHECK(card.first != nullptr);
        GP_CHECK(card.second != nullptr);
        GP_CHECK(card.first[0] != '\0');
    }

    GP_CHECK(gp::story_ending().first[0] != '\0');
}

GP_TEST(an_out_of_range_card_falls_back_to_the_first)
{
    GP_CHECK(gp::story_before(-1).first == gp::story_before(0).first);
    GP_CHECK(gp::story_before(9999).first == gp::story_before(0).first);
}

GP_TEST(no_story_line_runs_off_the_screen)
{
    // The 8x16 font averages about 7 pixels a character across a 240 pixel screen.
    constexpr int line_limit = 30;

    for (int index = 0; index < gp::story_card_count(); ++index)
    {
        gp::story_card card = gp::story_before(index);
        int first = 0;
        int second = 0;

        while (card.first[first] != '\0')
        {
            ++first;
        }

        while (card.second[second] != '\0')
        {
            ++second;
        }

        GP_CHECK(first <= line_limit);
        GP_CHECK(second <= line_limit);
    }
}
