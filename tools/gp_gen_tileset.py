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
import struct

TILE_SIZE = 16

# Index 0 is transparent everywhere in Butano and is keyed out in the .tsx.
PALETTE = [
    (255, 0, 255),  # 0 transparent
    (24, 26, 38),  # 1 wall body
    (46, 52, 72),  # 2 wall highlight
    (12, 14, 22),  # 3 wall shadow
    (58, 62, 54),  # 4 floor body
    (72, 78, 68),  # 5 floor fleck
    (40, 44, 38),  # 6 floor seam
    (90, 96, 84),  # 7 spare
    (120, 96, 72),  # 8 spare
    (150, 128, 96),  # 9 spare
    (96, 120, 128),  # 10 spare
    (128, 156, 168),  # 11 spare
    (168, 96, 96),  # 12 spare
    (96, 168, 120),  # 13 spare
    (200, 200, 200),  # 14 spare
    (255, 255, 255),  # 15 spare
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


def pack_4bpp(rows):
    """Two pixels per byte, high nibble first. Row width is a multiple of 8, so no padding."""
    packed = bytearray()

    for row in rows:
        for index in range(0, len(row), 2):
            packed.append(((row[index] & 0x0F) << 4) | (row[index + 1] & 0x0F))

    return bytes(packed)


def write_bmp(path, rows):
    width = len(rows[0])
    height = len(rows)

    if width % 8 or height % 8:
        raise SystemExit(f"{path}: {width}x{height} must be a multiple of 8 on both axes")

    # BMP stores rows bottom-up.
    pixels = pack_4bpp(list(reversed(rows)))
    palette = b"".join(struct.pack("<BBBB", b, g, r, 0) for r, g, b in PALETTE)

    offset = 14 + 40 + len(palette)
    header = struct.pack("<2sIHHI", b"BM", offset + len(pixels), 0, 0, offset)
    info = struct.pack("<IiiHHIIiiII", 40, width, height, 1, 4, 0, len(pixels), 2835, 2835, 16, 0)

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + info + palette + pixels)
    print(f"{path}: {width}x{height}, 4bpp, {len(PALETTE)} colours, {offset + len(pixels)} bytes")


def main():
    parser = argparse.ArgumentParser(description="Generate the placeholder map tileset.")
    parser.add_argument("--out", default="maps/chirp_tiles.bmp", help="output BMP path")
    args = parser.parse_args()

    write_bmp(pathlib.Path(args.out), floor_tile() + wall_tile())


if __name__ == "__main__":
    main()
