#!/usr/bin/env python3
"""Generate the placeholder map tileset.

Writes a 4bpp, 16-colour BMP with a 40-byte BITMAPINFOHEADER, which is what both Tiled and
Butano's bmp.py accept. Most image editors emit V4 or V5 headers, which Butano rejects, so the
tileset is generated rather than drawn until there is real art.

    python tools/gp_gen_tileset.py

Tile order is the tileset's, so a gid of 1 is floor and 2 is wall. The collision layer only cares
whether a cell is non-zero, but keeping the order stable means a map still reads correctly in
Tiled after a regeneration.
"""

import argparse
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

from gp_bmp import TRANSPARENT, write_bmp  # noqa: E402

TILE_SIZE = 16

# Index 0 is transparent everywhere in Butano and is keyed out in the .tsx.
PALETTE = [
    TRANSPARENT,
    (24, 26, 38),
    (46, 52, 72),
    (12, 14, 22),
    (58, 62, 54),
    (72, 78, 68),
    (40, 44, 38),
]


def floor_tile():
    """Flat ground with a seam grid and a couple of flecks, so movement reads at a glance."""
    rows = []

    for y in range(TILE_SIZE):
        row = []

        for x in range(TILE_SIZE):
            if x == 0 or y == 0:
                row.append(6)
            elif (x * 7 + y * 3) % 23 == 0:
                row.append(5)
            else:
                row.append(4)

        rows.append(row)

    return rows


def wall_tile():
    """Solid block, lit along the top and left, so a wall never reads as walkable."""
    rows = []

    for y in range(TILE_SIZE):
        row = []

        for x in range(TILE_SIZE):
            if y == 0 or x == 0:
                row.append(2)
            elif y == TILE_SIZE - 1 or x == TILE_SIZE - 1:
                row.append(3)
            else:
                row.append(1)

        rows.append(row)

    return rows


def main():
    parser = argparse.ArgumentParser(description="Generate the placeholder map tileset.")
    parser.add_argument("--out", default="maps/chirp_tiles.bmp", help="output BMP path")
    args = parser.parse_args()

    width, height = write_bmp(pathlib.Path(args.out), floor_tile() + wall_tile(), PALETTE)
    print(f"{args.out}: {width}x{height}, 4bpp, 2 tiles")


if __name__ == "__main__":
    main()
