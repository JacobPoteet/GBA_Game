/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_CHIRP_PLAYER_H
#define GP_CHIRP_PLAYER_H

#include "gp_signature.h"

namespace gp
{

/**
 * Plays a motif one note at a time.
 *
 * A chirp is one sample per clan played at a rate of 2^(semitone/12), so the clan picks the timbre
 * and the family picks the pitch. Notes are spaced out over frames rather than fired at once, which
 * is what makes a caller sound like a question and a responder like its answer.
 *
 * Call `update()` once a frame and `play()` to start a chirp. Starting a second chirp cancels the
 * first, so a player mashing B hears the newest one rather than a pile.
 */
class chirp_player
{

public:
    /** Frames each note is held for. */
    static constexpr int note_frames = 7;

    void play(signature value);

    void update();

    [[nodiscard]] bool busy() const
    {
        return _note < motif_length;
    }

private:
    motif _motif;
    int _clan = 0;
    int _note = motif_length;
    int _countdown = 0;

    void _play_current_note() const;
};

} // namespace gp

#endif
