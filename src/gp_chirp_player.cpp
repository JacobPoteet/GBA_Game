/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_chirp_player.h"

#include "bn_fixed.h"
#include "bn_sound.h"
#include "bn_sound_items.h"

namespace gp
{

namespace
{

constexpr int lowest_semitone = -12;
constexpr int highest_semitone = 24;
constexpr int rate_count = highest_semitone - lowest_semitone + 1;

/**
 * 2^(n/12) in bn::fixed's raw 12 bit fraction, for semitones -12 to +24.
 *
 * A table rather than a pow() call: the GBA has no FPU, and every rate the game can ask for is
 * known at build time. Index with `semitone - lowest_semitone`.
 */
constexpr int rate_data[rate_count] = {
        2048, 2170, 2299, 2435, 2580,  2734,  2896,  3069,  3251,  3444,  3649,  3866,  // -12..-1
        4096, 4340, 4598, 4871, 5161,  5468,  5793,  6137,  6502,  6889,  7298,  7732,  // 0..11
        8192, 8679, 9195, 9742, 10321, 10935, 11585, 12274, 13004, 13777, 14596, 15464, // 12..23
        16384                                                                           // 24
};

[[nodiscard]] bn::fixed rate_for(int semitone)
{
    int index = semitone - lowest_semitone;

    if (index < 0)
    {
        index = 0;
    }
    else if (index >= rate_count)
    {
        index = rate_count - 1;
    }

    return bn::fixed::from_data(rate_data[index]);
}

[[nodiscard]] bn::sound_item sample_for(int clan)
{
    switch (clan)
    {

    case 0:
        return bn::sound_items::gp_chirp_0;

    case 1:
        return bn::sound_items::gp_chirp_1;

    case 2:
        return bn::sound_items::gp_chirp_2;

    default:
        return bn::sound_items::gp_chirp_3;
    }
}

} // namespace

void chirp_player::play(signature value)
{
    _motif = motif_for(value);
    _clan = int(value.clan);
    _note = 0;
    _countdown = 0;
}

void chirp_player::update()
{
    if (!busy())
    {
        return;
    }

    if (_countdown > 0)
    {
        --_countdown;
        return;
    }

    _play_current_note();
    _countdown = note_frames;
    ++_note;
}

void chirp_player::_play_current_note() const
{
    bn::sound::play(sample_for(_clan), bn::fixed(0.7), rate_for(_motif.semitones[_note]), 0);
}

} // namespace gp
