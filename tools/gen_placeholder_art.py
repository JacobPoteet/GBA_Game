#!/usr/bin/env python3
"""
Regenerate the placeholder sprite sheets in graphics/.

Butano's asset pipeline (third_party/butano/butano/tools/bmp.py) is strict about its input:
  * BITMAPINFOHEADER only - a 40 byte DIB header, never the 108/124 byte V4/V5 variants
  * 4 or 8 bits per pixel, no compression
  * width and height both divisible by 8
A 4bpp image with a 16 entry palette is the simplest thing that satisfies all of it, and it is
what Butano's own bundled assets use, so that is what this script emits.

Palette index 0 is the transparent color for sprites.

Usage:  python tools/gen_placeholder_art.py
"""

import os
import struct

GRAPHICS_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'graphics')

TRANSPARENT = (255, 0, 255)

# Index -> RGB. Kept at exactly 16 entries: bmp.py derives the bit depth from the palette size.
PALETTE = [
    TRANSPARENT,        # 0  transparent
    (208, 70, 72),      # 1  red
    (120, 32, 40),      # 2  red shadow
    (96, 168, 88),      # 3  green
    (48, 96, 48),       # 4  green shadow
    (88, 128, 200),     # 5  blue
    (40, 64, 120),      # 6  blue shadow
    (216, 184, 88),     # 7  yellow
    (136, 104, 32),     # 8  yellow shadow
    (240, 240, 240),    # 9  white
    (24, 24, 32),       # 10 near black
    (0, 0, 0),          # 11 unused
    (0, 0, 0),          # 12 unused
    (0, 0, 0),          # 13 unused
    (0, 0, 0),          # 14 unused
    (0, 0, 0),          # 15 unused
]

TILE_SIZE = 16

# (fill index, shadow index) per tile kind, matching gp::tile_kind in include/gp_tile_kind.h.
TILE_COLORS = [(1, 2), (3, 4), (5, 6), (7, 8)]


def write_bmp_4bpp(path, width, height, rows):
    """Write a 4bpp uncompressed BMP. `rows` is top-down; BMP stores bottom-up."""
    if width % 8 or height % 8:
        raise ValueError('width and height must be divisible by 8')

    row_bytes = width // 2
    if row_bytes % 4:
        row_bytes += 4 - (row_bytes % 4)

    pixel_data = bytearray()

    for row in reversed(rows):
        packed = bytearray()

        for x in range(0, width, 2):
            packed.append(((row[x] & 0x0F) << 4) | (row[x + 1] & 0x0F))

        packed.extend(b'\x00' * (row_bytes - len(packed)))
        pixel_data.extend(packed)

    pixels_offset = 14 + 40 + (16 * 4)
    file_size = pixels_offset + len(pixel_data)

    header = bytearray()
    header += b'BM'
    header += struct.pack('<IHHI', file_size, 0, 0, pixels_offset)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 4, 0, len(pixel_data), 0, 0, 16, 16)

    for r, g, b in PALETTE:
        header += struct.pack('<BBBB', b, g, r, 0)

    with open(path, 'wb') as bmp_file:
        bmp_file.write(bytes(header) + bytes(pixel_data))

    print('wrote {} ({}x{}, {} bytes)'.format(os.path.basename(path), width, height, file_size))


def write_json(path, height):
    with open(path, 'w', newline='\n') as json_file:
        json_file.write('{\n    "type": "sprite",\n    "height": %d\n}\n' % height)


def build_tile_frame(fill, shadow):
    """A beveled square with a one pixel transparent margin, so tiles read as separate pieces."""
    frame = []

    for y in range(TILE_SIZE):
        row = []

        for x in range(TILE_SIZE):
            if x == 0 or y == 0 or x == TILE_SIZE - 1 or y == TILE_SIZE - 1:
                row.append(0)
            elif x == 1 or y == 1:
                row.append(fill)
            elif x == TILE_SIZE - 2 or y == TILE_SIZE - 2:
                row.append(shadow)
            else:
                row.append(fill)

        frame.append(row)

    return frame


def build_cursor_frame():
    """Corner brackets only, so the tile underneath stays visible."""
    arm = 5
    frame = [[0] * TILE_SIZE for _ in range(TILE_SIZE)]

    for i in range(arm):
        for cx, cy in ((0, 0), (TILE_SIZE - 1, 0), (0, TILE_SIZE - 1), (TILE_SIZE - 1, TILE_SIZE - 1)):
            step = i if cx == 0 else -i
            frame[cy][cx + step] = 9
            step = i if cy == 0 else -i
            frame[cy + step][cx] = 9

    return frame


def main():
    os.makedirs(GRAPHICS_DIR, exist_ok=True)

    tile_rows = []

    for fill, shadow in TILE_COLORS:
        tile_rows.extend(build_tile_frame(fill, shadow))

    write_bmp_4bpp(os.path.join(GRAPHICS_DIR, 'gp_tile.bmp'), TILE_SIZE, len(tile_rows), tile_rows)
    write_json(os.path.join(GRAPHICS_DIR, 'gp_tile.json'), TILE_SIZE)

    cursor_rows = build_cursor_frame()
    write_bmp_4bpp(os.path.join(GRAPHICS_DIR, 'gp_cursor.bmp'), TILE_SIZE, len(cursor_rows), cursor_rows)
    write_json(os.path.join(GRAPHICS_DIR, 'gp_cursor.json'), TILE_SIZE)


if __name__ == '__main__':
    main()
