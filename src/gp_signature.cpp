/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#include "gp_signature.h"

namespace gp
{

namespace
{
/**
 * Minor pentatonic degrees. Any two of them are far enough apart that a player can tell one root
 * from another after hearing each once.
 */
constexpr int8_t family_roots[families_per_clan] = {0, 3, 7, 10};

constexpr int8_t caller_degrees[motif_length] = {0, 4, 7, 11};
constexpr int8_t responder_degrees[motif_length] = {12, 7, 4, 0};
} // namespace

int family_root_semitone(int family)
{
    if (family < 0 || family >= families_per_clan)
    {
        return 0;
    }

    return family_roots[family];
}

motif motif_for(signature value)
{
    const int8_t* degrees = value.part == role::caller ? caller_degrees : responder_degrees;
    int root = family_root_semitone(value.family);
    motif result;

    for (int index = 0; index < motif_length; ++index)
    {
        result.semitones[index] = int8_t(root + degrees[index]);
    }

    return result;
}

} // namespace gp
