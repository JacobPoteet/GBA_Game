/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_SOLVER_H
#define GP_SOLVER_H

#include <vector>

#include "gp_level_data.h"

namespace gp
{

/** One pairing, in the order a player would make it. */
struct solve_step
{
    int first = no_character;
    int second = no_character;
};

struct solve_result
{
    bool solvable = false;

    /** The character the level holds for the player, or `no_character`. Only the last level has one. */
    int player_match = no_character;

    std::vector<solve_step> steps;
};

/**
 * Works out whether a level can be finished.
 *
 * Walks every ordering of pairings, pruning orderings it has already failed. A pairing counts only
 * when the player can stand next to both characters in the configuration that ordering produces,
 * which is what makes blocker ordering matter: some pairs stay out of reach until an earlier pair
 * leaves.
 *
 * The player's own match is not a pairing. They are left out of the search and checked once at the
 * end, when everybody else has gone, because that is the only moment the player can reach them.
 *
 * This lives in `tests/` on purpose. CI runs it over every level, and the ROM never carries it.
 */
[[nodiscard]] solve_result solve(const level_spec& spec);

} // namespace gp

#endif
