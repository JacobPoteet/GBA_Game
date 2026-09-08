/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_SIGNATURE_H
#define GP_SIGNATURE_H

#include <cstdint>

namespace gp
{

/**
 * Which half of a pair a character is. A caller asks, a responder answers.
 *
 * Sensed as palette temperature and posture: callers are warm and lean open, responders are cool
 * and curl closed.
 */
enum class role : uint8_t
{
    caller = 0,
    responder
};

[[nodiscard]] constexpr role opposite(role value)
{
    return value == role::caller ? role::responder : role::caller;
}

/** Clans pick the base sample and the visible pattern. */
constexpr int clan_count = 4;

/** Families sit inside a clan and are told apart by ear alone. */
constexpr int families_per_clan = 4;

/**
 * What a character is, and the only thing that decides who they pair with.
 *
 * `clan` is visible as a pattern, so it narrows the field. `family` is audible as the root note of
 * the chirp and nothing else, so the player has to listen to separate the families inside one clan.
 */
struct signature
{
    uint8_t clan = 0;
    uint8_t family = 0;
    role part = role::caller;

    [[nodiscard]] friend constexpr bool operator==(signature a, signature b) = default;
};

/**
 * The match rule. Same clan, same family, opposite roles.
 */
[[nodiscard]] constexpr bool matches(signature a, signature b)
{
    return a.clan == b.clan && a.family == b.family && a.part != b.part;
}

/** Notes in one chirp. */
constexpr int motif_length = 4;

/**
 * A chirp as semitone offsets from the clan's base sample pitch. Plain integers, so a test can
 * compare two motifs without linking a sound engine. The Butano layer turns each offset into a
 * playback rate.
 */
struct motif
{
    int8_t semitones[motif_length] = {};

    [[nodiscard]] friend constexpr bool operator==(const motif& a, const motif& b) = default;
};

/**
 * The root note a family sits on, in semitones. Spaced so neighbouring families stay apart by ear.
 */
[[nodiscard]] int family_root_semitone(int family);

/**
 * The chirp a signature performs.
 *
 * A caller climbs and stops on the major seventh, which leaves the phrase hanging. A responder
 * falls from the octave and lands on the root. Play a true pair back to back and the two halves
 * complete one arch, which is what a successful match sounds like.
 */
[[nodiscard]] motif motif_for(signature value);

} // namespace gp

#endif
