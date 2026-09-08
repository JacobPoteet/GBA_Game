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
 * This lives in `tests/` on purpose. CI runs it over every level, and the ROM never carries it.
 */
[[nodiscard]] solve_result solve(const level_spec& spec);

} // namespace gp

#endif
