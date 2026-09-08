#!/usr/bin/env python3
"""Generate one base chirp sample per clan.

    python tools/gp_gen_chirps.py

Writes audio/gp_chirp_0.wav through gp_chirp_3.wav as 8 bit unsigned mono at 22050 Hz, which is
what Butano's Maxmod backend takes, and what Butano's own audio example ships.

There is one sample per clan, not one per character. A character's chirp is that sample played at
a rate of 2^(semitone/12), so the clan picks the timbre and the family picks the pitch. See the
wiki's Chirps note.

Every sample sits at the same reference pitch, so semitone 0 sounds the same across clans and only
the colour of the tone differs.
"""

import argparse
import math
import pathlib
import struct
import wave

RATE = 22050
REFERENCE_HZ = 440.0
SECONDS = 0.13

# One per clan. The shape is what tells two clans apart when they sit on the same note.
SHAPES = ("sine", "reed", "hollow", "bright")


def wave_sample(shape, phase):
    """`phase` runs 0 to 1 across one cycle."""
    if shape == "sine":
        return math.sin(2 * math.pi * phase)

    if shape == "reed":
        # A narrow pulse, which reads as reedy rather than as a plain square.
        return 1.0 if phase < 0.25 else -0.55

    if shape == "hollow":
        return 4 * abs(phase - 0.5) - 1

    # Bright: the fundamental with an octave and a fifth stacked under it.
    return (math.sin(2 * math.pi * phase) * 0.6 +
            math.sin(4 * math.pi * phase) * 0.3 +
            math.sin(6 * math.pi * phase) * 0.1)


def envelope(position, total):
    """Quick attack, long decay. Both ends reach zero so a note never clicks."""
    attack = max(1, int(total * 0.04))

    if position < attack:
        return position / attack

    remaining = (total - position) / (total - attack)
    return remaining ** 1.6


def render(shape):
    total = int(RATE * SECONDS)
    frames = bytearray()

    for position in range(total):
        phase = (position * REFERENCE_HZ / RATE) % 1.0
        value = wave_sample(shape, phase) * envelope(position, total) * 0.82
        frames.append(max(0, min(255, int(round(value * 127)) + 128)))

    return bytes(frames)


def write_wav(path, frames):
    path.parent.mkdir(parents=True, exist_ok=True)

    with wave.open(str(path), "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(1)
        handle.setframerate(RATE)
        handle.writeframes(frames)

    return len(frames)


def main():
    parser = argparse.ArgumentParser(description="Generate the placeholder chirp samples.")
    parser.add_argument("--out", default="audio", help="output directory")
    args = parser.parse_args()

    out = pathlib.Path(args.out)

    for clan, shape in enumerate(SHAPES):
        path = out / f"gp_chirp_{clan}.wav"
        count = write_wav(path, render(shape))
        print(f"{path}: {shape}, {count} frames, {count / RATE:.3f}s, 8 bit mono {RATE} Hz")


if __name__ == "__main__":
    main()
