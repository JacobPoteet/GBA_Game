#!/usr/bin/env python3
"""Generate the placeholder music.

    python tools/gp_gen_music.py

Writes audio/gp_theme.mod as a four channel ProTracker module, which is what Butano's Maxmod
backend takes. Three channels hold a slow chord pad and the fourth holds the root underneath.

The format is fixed and old, so it is written out by hand rather than pulled from a library:

    0     20 bytes    title
    20    31 x 30     sample headers: 22 name, 2 length in words, 1 finetune, 1 volume,
                      2 repeat offset in words, 2 repeat length in words
    950   1           song length in patterns
    951   1           restart position
    952   128         pattern order
    1080  4           "M.K.", the four channel marker
    1084  ...         patterns, 64 rows x 4 channels x 4 bytes
    ...               sample data, signed 8 bit

Every sample is a single cycle looped forever, so pitch comes entirely from the note period and a
32 byte sample stays in tune wherever it is played.
"""

import argparse
import math
import pathlib
import struct

CHANNELS = 4
ROWS = 64
CYCLE = 32

# Amiga periods. Larger is lower. Octave 1 is the low one here.
PERIOD = {
    "C-1": 856, "D-1": 762, "E-1": 678, "F-1": 640, "G-1": 570, "A-1": 508, "B-1": 453,
    "C-2": 428, "D-2": 381, "E-2": 339, "F-2": 320, "G-2": 285, "A-2": 254, "B-2": 226,
    "C-3": 214, "D-3": 190, "E-3": 170, "G-3": 143,
}

# Four bars of A minor, F, C, G. Three voices and a root.
CHORDS = [
    (("A-1", "C-2", "E-2"), "A-1"),
    (("F-1", "A-1", "C-2"), "F-1"),
    (("C-2", "E-2", "G-2"), "C-1"),
    (("G-1", "B-1", "D-2"), "G-1"),
]

PAD_SAMPLE = 1
BASS_SAMPLE = 2

# Higher is slower. The default is 6.
SPEED = 10


def single_cycle(shape):
    """One cycle of a waveform as signed 8 bit, sized so a loop is seamless."""
    data = bytearray()

    for index in range(CYCLE):
        phase = index / CYCLE

        if shape == "pad":
            value = math.sin(2 * math.pi * phase) * 0.72 + math.sin(4 * math.pi * phase) * 0.18
        else:
            # A softened square, which keeps a root note audible under the pad without buzzing.
            value = (0.66 if phase < 0.5 else -0.66) * 0.5 + math.sin(2 * math.pi * phase) * 0.42

        data.append(int(round(max(-1.0, min(1.0, value)) * 100)) & 0xFF)

    return bytes(data)


def sample_header(name, data, volume):
    words = len(data) // 2
    # A repeat length above one word means the sample loops, which is what holds a pad note.
    return (name.encode("ascii").ljust(22, b"\0") +
            struct.pack(">H", words) +
            bytes([0, volume]) +
            struct.pack(">HH", 0, words))


def note(sample, period, effect=0, param=0):
    return bytes([
        (sample & 0xF0) | ((period >> 8) & 0x0F),
        period & 0xFF,
        ((sample & 0x0F) << 4) | (effect & 0x0F),
        param,
    ])


EMPTY = note(0, 0)


def pattern():
    """One pattern: a chord every sixteen rows, with the root restruck halfway through each."""
    rows = [[EMPTY] * CHANNELS for _ in range(ROWS)]

    for index, (voices, root) in enumerate(CHORDS):
        row = index * 16

        for channel, name in enumerate(voices):
            rows[row][channel] = note(PAD_SAMPLE, PERIOD[name])

        rows[row][3] = note(BASS_SAMPLE, PERIOD[root])
        rows[row + 8][3] = note(BASS_SAMPLE, PERIOD[root])

    # Set the tempo once, on the first row, in a channel that is already striking a note.
    rows[0][0] = note(PAD_SAMPLE, PERIOD[CHORDS[0][0][0]], 0x0F, SPEED)

    return b"".join(b"".join(row) for row in rows)


def build():
    pad = single_cycle("pad")
    bass = single_cycle("bass")

    out = bytearray()
    out += b"chirp".ljust(20, b"\0")
    out += sample_header("pad", pad, 38)
    out += sample_header("root", bass, 30)
    out += bytes(30) * 29  # the 29 sample slots this module does not use

    out += bytes([1, 127])          # one pattern, restart at the conventional 127
    out += bytes([0]) + bytes(127)  # the order table plays pattern 0
    out += b"M.K."
    out += pattern()
    out += pad
    out += bass
    return bytes(out)


def check(data):
    """Reads back the parts that would silently break the build if they were wrong."""
    assert len(data) >= 1084, f"header is {len(data)} bytes"
    assert data[1080:1084] == b"M.K.", f"missing M.K. marker: {data[1080:1084]!r}"
    assert data[950] == 1, f"song length is {data[950]}"
    expected = 1084 + (ROWS * CHANNELS * 4) + (CYCLE * 2)
    assert len(data) == expected, f"{len(data)} bytes, expected {expected}"

    for slot in range(31):
        at = 20 + slot * 30
        words = struct.unpack(">H", data[at + 22:at + 24])[0]
        volume = data[at + 25]
        assert volume <= 64, f"sample {slot + 1} volume {volume}"

        if slot < 2:
            assert words == CYCLE // 2, f"sample {slot + 1} is {words} words"


def main():
    parser = argparse.ArgumentParser(description="Generate the placeholder music.")
    parser.add_argument("--out", default="audio/gp_theme.mod", help="output module path")
    args = parser.parse_args()

    data = build()
    check(data)

    path = pathlib.Path(args.out)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)
    print(f"{path}: {len(data)} bytes, {CHANNELS} channels, 1 pattern, {ROWS} rows, speed {SPEED}")


if __name__ == "__main__":
    main()
