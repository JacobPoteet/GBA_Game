#!/usr/bin/env python3
"""Generate the placeholder game art.

    python tools/gp_gen_art.py

Writes four asset pairs into graphics/:

    gp_bg_tiles      8x8 background tiles: void, floor, wall
    gp_bg_palette    the 16 colours those tiles use
    gp_character     16x16 sprites, one frame per clan and role
    gp_player        16x16 sprites, one frame per facing

Two couplings this file has to hold up, because nothing checks the art side automatically:

  * Character frame order is `clan * 2 + role`, which is what gp::signature_graphics_index returns.
  * Player frame order is gp::direction: up, down, left, right.

The art carries the two things the rules depend on and nothing else: a per-clan pattern, and a warm
or cool palette telling a caller from a responder. See the wiki's Signatures note.
"""

import argparse
import math
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

from gp_bmp import TRANSPARENT, blank, write_bmp, write_json  # noqa: E402

CLAN_COUNT = 4
ROLE_COUNT = 2
TILE = 16

# Backgrounds and sprites carry separate palettes, so each list is its own 16 colours.
BG_PALETTE = [
    TRANSPARENT,     # 0 void, shows the backdrop
    (58, 62, 54),    # 1 floor
    (48, 52, 46),    # 2 floor seam
    (70, 74, 64),    # 3 floor fleck
    (30, 32, 44),    # 4 wall
    (54, 58, 78),    # 5 wall lit edge
    (18, 19, 28),    # 6 wall shadow
]

SPRITE_PALETTE = [
    TRANSPARENT,     # 0
    (16, 16, 24),    # 1 outline
    (150, 78, 52),   # 2 warm dark
    (206, 118, 74),  # 3 warm mid
    (240, 172, 116),  # 4 warm light
    (44, 82, 116),   # 5 cool dark
    (72, 126, 168),  # 6 cool mid
    (128, 184, 216),  # 7 cool light
    (250, 220, 150),  # 8 warm pattern
    (196, 240, 250),  # 9 cool pattern
    (250, 250, 240),  # 10 eye white
]

PLAYER_PALETTE = [
    TRANSPARENT,     # 0
    (16, 16, 24),    # 1 outline
    (86, 64, 120),   # 2 body dark
    (128, 100, 172),  # 3 body mid
    (176, 152, 214),  # 4 body light
    (250, 250, 240),  # 5 eye white
    (240, 196, 128),  # 6 scarf
]

WARM = (2, 3, 4, 8)
COOL = (5, 6, 7, 9)


# ---------------------------------------------------------------------------------------------
# Background tiles


def bg_tiles():
    """Three 8x8 tiles laid out in one row: void, floor, wall."""
    rows = blank(24, 8)

    for y in range(8):
        for x in range(8):
            # Floor: a seam along the top and left, a fleck here and there.
            if x == 0 or y == 0:
                floor = 2
            elif (x * 5 + y * 3) % 11 == 0:
                floor = 3
            else:
                floor = 1

            rows[y][8 + x] = floor

            # Wall: lit along the top, shadowed along the bottom.
            if y == 0:
                wall = 5
            elif y >= 6:
                wall = 6
            else:
                wall = 4

            rows[y][16 + x] = wall

    return rows


# ---------------------------------------------------------------------------------------------
# Characters


def clan_pattern(clan, x, y, cx, cy):
    """Each clan wears a different marking. This is the visible half of a signature."""
    # Four textures, not four variations on one: bands one way, bands the other way, a grid of
    # dots, and a ring. At 16 pixels a subtle difference is no difference.
    if clan == 0:
        return (y % 5) < 2                                    # thick horizontal bands
    if clan == 1:
        return (x % 3 == 0) and (y % 3 == 0)                  # dot grid
    if clan == 2:
        return ((x + y) % 5) < 2                              # diagonal bands
    ring = math.hypot(x - cx, y - cy)
    return 2.4 <= ring <= 3.4                                 # one bold ring


def character_frame(clan, role):
    """A blob that leans forward when it asks and curls back when it answers."""
    rows = blank(TILE, TILE)
    dark, mid, light, accent = WARM if role == 0 else COOL
    caller = role == 0
    cx, cy = 7.5, 8.5

    for y in range(TILE):
        for x in range(TILE):
            # Callers lean right and stand tall; responders lean left and hunch.
            lean = (TILE - y) * 0.16
            ox = cx + (lean if caller else -lean) - (0.0 if caller else 0.5)
            radius = 5.4 if caller else 5.1
            distance = math.hypot((x - ox) * 1.05, (y - cy) * (0.92 if caller else 1.02))

            if distance > radius + 0.9:
                continue

            if distance > radius:
                rows[y][x] = 1
                continue

            shade = mid

            if (y - cy) < -1.5:
                shade = light
            elif (y - cy) > 2.2:
                shade = dark

            if clan_pattern(clan, x, y, cx, cy):
                shade = accent

            rows[y][x] = shade

    # A tuft, up for a caller and drooping for a responder, so the role reads without colour.
    if caller:
        for y in range(0, 3):
            rows[y][9] = light
            rows[y][10] = 1
    else:
        for x in range(4, 7):
            rows[2][x] = dark
            rows[3][x] = 1

    # One eye, facing the way the body leans.
    eye_x = 10 if caller else 5
    rows[8][eye_x] = 10
    rows[8][eye_x + (1 if caller else -1)] = 1
    return rows


def character_sheet():
    sheet = []

    for clan in range(CLAN_COUNT):
        for role in range(ROLE_COUNT):
            sheet.extend(character_frame(clan, role))

    return sheet


# ---------------------------------------------------------------------------------------------
# Player


def player_frame(facing):
    """Frame order is gp::direction: up, down, left, right."""
    rows = blank(TILE, TILE)
    cx, cy = 7.5, 8.5

    for y in range(TILE):
        for x in range(TILE):
            distance = math.hypot((x - cx) * 1.02, (y - cy) * 0.94)

            if distance > 5.9:
                continue

            if distance > 5.0:
                rows[y][x] = 1
                continue

            shade = 3

            if (y - cy) < -1.8:
                shade = 4
            elif (y - cy) > 2.4:
                shade = 2

            rows[y][x] = shade

    # A scarf, so the player never reads as one of the characters.
    for x in range(4, 12):
        if rows[12][x]:
            rows[12][x] = 6
        if rows[13][x]:
            rows[13][x] = 6

    if facing == 0:      # up, the back of the head
        pass
    elif facing == 1:    # down
        for eye_x in (5, 10):
            rows[7][eye_x] = 5
            rows[8][eye_x] = 1
    elif facing == 2:    # left
        rows[7][4] = 5
        rows[8][4] = 1
        rows[7][5] = 1
    else:                # right
        rows[7][11] = 5
        rows[8][11] = 1
        rows[7][10] = 1

    return rows


def player_sheet():
    sheet = []

    for facing in range(4):
        sheet.extend(player_frame(facing))

    return sheet


# ---------------------------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(description="Generate the placeholder game art.")
    parser.add_argument("--out", default="graphics", help="output directory")
    args = parser.parse_args()

    out = pathlib.Path(args.out)
    written = []

    size = write_bmp(out / "gp_bg_tiles.bmp", bg_tiles(), BG_PALETTE)
    write_json(out / "gp_bg_tiles.json", {"type": '"regular_bg_tiles"', "bpp_mode": '"bpp_4"'})
    written.append(("gp_bg_tiles", size, "3 tiles"))

    size = write_bmp(out / "gp_bg_palette.bmp", blank(8, 8, 1), BG_PALETTE)
    write_json(out / "gp_bg_palette.json",
               {"type": '"bg_palette"', "bpp_mode": '"bpp_4"', "colors_count": 16})
    written.append(("gp_bg_palette", size, "16 colours"))

    size = write_bmp(out / "gp_character.bmp", character_sheet(), SPRITE_PALETTE)
    write_json(out / "gp_character.json", {"type": '"sprite"', "height": TILE})
    written.append(("gp_character", size, f"{CLAN_COUNT * ROLE_COUNT} frames, clan * 2 + role"))

    size = write_bmp(out / "gp_player.bmp", player_sheet(), PLAYER_PALETTE)
    write_json(out / "gp_player.json", {"type": '"sprite"', "height": TILE})
    written.append(("gp_player", size, "4 frames, gp::direction order"))

    for name, (width, height), note in written:
        print(f"{name}: {width}x{height}, {note}")


if __name__ == "__main__":
    main()
