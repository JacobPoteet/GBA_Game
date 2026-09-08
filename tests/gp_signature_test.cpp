/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_test_framework.h"

#include "gp_signature.h"

namespace
{

gp::signature make(int clan, int family, gp::role part)
{
    gp::signature value;
    value.clan = uint8_t(clan);
    value.family = uint8_t(family);
    value.part = part;
    return value;
}

} // namespace

GP_TEST(a_caller_matches_the_responder_of_the_same_family)
{
    GP_CHECK(gp::matches(make(1, 2, gp::role::caller), make(1, 2, gp::role::responder)));
}

GP_TEST(matching_does_not_care_which_side_is_asked_first)
{
    gp::signature caller = make(1, 2, gp::role::caller);
    gp::signature responder = make(1, 2, gp::role::responder);
    GP_CHECK_EQ(gp::matches(caller, responder), gp::matches(responder, caller));
}

GP_TEST(two_callers_never_match)
{
    GP_CHECK(!gp::matches(make(1, 2, gp::role::caller), make(1, 2, gp::role::caller)));
}

GP_TEST(the_same_family_in_a_different_clan_never_matches)
{
    GP_CHECK(!gp::matches(make(0, 2, gp::role::caller), make(1, 2, gp::role::responder)));
}

GP_TEST(the_near_miss_is_the_same_clan_and_a_different_family)
{
    GP_CHECK(!gp::matches(make(1, 0, gp::role::caller), make(1, 3, gp::role::responder)));
}

GP_TEST(every_family_sits_on_its_own_root_note)
{
    for (int first = 0; first < gp::families_per_clan; ++first)
    {
        for (int second = first + 1; second < gp::families_per_clan; ++second)
        {
            GP_CHECK(gp::family_root_semitone(first) != gp::family_root_semitone(second));
        }
    }
}

GP_TEST(a_family_outside_the_range_falls_back_to_the_root)
{
    GP_CHECK_EQ(gp::family_root_semitone(-1), 0);
    GP_CHECK_EQ(gp::family_root_semitone(gp::families_per_clan), 0);
}

GP_TEST(a_caller_motif_climbs_and_a_responder_motif_falls)
{
    gp::motif caller = gp::motif_for(make(0, 1, gp::role::caller));
    gp::motif responder = gp::motif_for(make(0, 1, gp::role::responder));

    for (int index = 1; index < gp::motif_length; ++index)
    {
        GP_CHECK(caller.semitones[index] > caller.semitones[index - 1]);
        GP_CHECK(responder.semitones[index] < responder.semitones[index - 1]);
    }
}

GP_TEST(a_responder_lands_on_the_family_root)
{
    for (int family = 0; family < gp::families_per_clan; ++family)
    {
        gp::motif value = gp::motif_for(make(0, family, gp::role::responder));
        GP_CHECK_EQ(int(value.semitones[gp::motif_length - 1]), gp::family_root_semitone(family));
    }
}

GP_TEST(a_caller_leaves_the_phrase_hanging_above_its_root)
{
    for (int family = 0; family < gp::families_per_clan; ++family)
    {
        gp::motif value = gp::motif_for(make(0, family, gp::role::caller));
        GP_CHECK_EQ(int(value.semitones[0]), gp::family_root_semitone(family));
        GP_CHECK(int(value.semitones[gp::motif_length - 1]) > gp::family_root_semitone(family));
    }
}

GP_TEST(two_families_in_one_clan_sound_different)
{
    gp::motif first = gp::motif_for(make(2, 0, gp::role::caller));
    gp::motif second = gp::motif_for(make(2, 1, gp::role::caller));
    GP_CHECK(!(first == second));
}

GP_TEST(the_clan_does_not_change_the_motif)
{
    GP_CHECK(gp::motif_for(make(0, 2, gp::role::caller)) == gp::motif_for(make(3, 2, gp::role::caller)));
}

GP_TEST(opposite_flips_the_role)
{
    GP_CHECK(gp::opposite(gp::role::caller) == gp::role::responder);
    GP_CHECK(gp::opposite(gp::role::responder) == gp::role::caller);
}
