#!/usr/bin/env python3
"""Write the one BMP flavour Butano accepts.

4bpp, a 16 colour table, uncompressed, and a 40 byte BITMAPINFOHEADER. Butano's bmp.py rejects the
V4 and V5 headers that most image editors write by default, which is why the placeholder art is
generated rather than drawn.

Both axes must be a multiple of 8. Palette index 0 is transparent everywhere in Butano.
"""

import pathlib
import struct

TRANSPARENT = (255, 0, 255)


def pad_palette(colors):
    """A 4bpp BMP needs exactly 16 entries, so short palettes get padded with black."""
    if len(colors) > 16:
        raise ValueError(f"{len(colors)} colours, a 4bpp palette holds 16")

    return list(colors) + [(0, 0, 0)] * (16 - len(colors))


def write_bmp(path, rows, palette):
    """`rows` is a top-down list of rows of palette indices."""
    path = pathlib.Path(path)
    height = len(rows)
    width = len(rows[0]) if height else 0

    if not height or not width:
        raise ValueError(f"{path}: empty image")

    if any(len(row) != width for row in rows):
        raise ValueError(f"{path}: rows are not all {width} wide")

    if width % 8 or height % 8:
        raise ValueError(f"{path}: {width}x{height} must be a multiple of 8 on both axes")

    # BMP stores rows bottom-up. 4bpp packs two pixels per byte, high nibble first, and a row of a
    # multiple of 8 pixels is already a multiple of 4 bytes, so no padding is needed.
    packed = bytearray()

    for row in reversed(rows):
        for index in range(0, width, 2):
            packed.append(((row[index] & 0x0F) << 4) | (row[index + 1] & 0x0F))

    table = b"".join(struct.pack("<BBBB", b, g, r, 0) for r, g, b in pad_palette(palette))
    offset = 14 + 40 + len(table)
    header = struct.pack("<2sIHHI", b"BM", offset + len(packed), 0, 0, offset)
    info = struct.pack("<IiiHHIIiiII", 40, width, height, 1, 4, 0, len(packed), 2835, 2835, 16, 0)

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(header + info + table + bytes(packed))
    return width, height


def write_json(path, body):
    path = pathlib.Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = ",\n".join(f'    "{key}": {value}' for key, value in body.items())
    path.write_text("{\n" + lines + "\n}\n", encoding="utf-8")


def blank(width, height, index=0):
    return [[index] * width for _ in range(height)]
