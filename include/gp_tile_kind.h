/*
 * Copyright (c) 2026 Jacob Poteet
 * MIT License, see LICENSE file.
 */

#ifndef GP_TILE_KIND_H
#define GP_TILE_KIND_H

#include <cstdint>

namespace gp
{

/**
 * A single cell's contents.
 *
 * This header is deliberately free of Butano includes so that it can be compiled by the host
 * unit tests as well as by the GBA toolchain. See docs/architecture.md.
 */
enum class tile_kind : uint8_t
{
    empty = 0,
    red,
    green,
    blue,
    yellow
};

/// Number of non-empty kinds. Must match the frame count of graphics/gp_tile.bmp.
constexpr int tile_kind_count = 4;

[[nodiscard]] constexpr bool is_empty(tile_kind kind)
{
    return kind == tile_kind::empty;
}

/**
 * Frame index of a kind inside the gp_tile sprite sheet.
 *
 * The ordering mirrors TILE_COLORS in tools/gen_placeholder_art.py; changing one without the
 * other silently swaps the on-screen colors.
 */
[[nodiscard]] constexpr int tile_kind_graphics_index(tile_kind kind)
{
    return static_cast<int>(kind) - 1;
}

/// Inverse of tile_kind_graphics_index. `index` is expected to be in [0, tile_kind_count).
[[nodiscard]] constexpr tile_kind tile_kind_from_graphics_index(int index)
{
    return static_cast<tile_kind>(index + 1);
}

/// Next non-empty kind, wrapping around. Used by the placeholder "cycle tile" input.
[[nodiscard]] constexpr tile_kind next_tile_kind(tile_kind kind)
{
    if (is_empty(kind))
    {
        return tile_kind::red;
    }

    return tile_kind_from_graphics_index((tile_kind_graphics_index(kind) + 1) % tile_kind_count);
}

} // namespace gp

#endif
