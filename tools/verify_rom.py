#!/usr/bin/env python3
"""
Validate the header of a built .gba ROM.

The build succeeding is not quite the same as the build being loadable: a truncated link, a
missing gbafix pass or a bad ROMTITLE all produce a file that make is perfectly happy with and
that no emulator will boot. This checks the parts of the GBA cartridge header that are
deterministic, so CI fails loudly instead of publishing a dead ROM.

Header layout (offsets into the ROM):
    0x000-0x003  entry point, an ARM branch, so byte 0x003 is always 0xEA
    0x004-0x09F  Nintendo logo, injected by the devkitARM crt0
    0x0A0-0x0AB  game title, 12 bytes, space padded
    0x0AC-0x0AF  game code, 4 bytes
    0x0B0-0x0B1  maker code
    0x0B2        fixed value, always 0x96
    0x0BD        header complement check

The logo is only checked for "not blank" rather than byte compared: its contents belong to the
toolchain, and pinning a copy here would just be a second thing to keep in sync.

Usage:  python tools/verify_rom.py gba_game.gba --title "GBA PUZZLE" --code PZLE
"""

import argparse
import os
import sys

HEADER_SIZE = 0xC0
MAX_ROM_SIZE = 32 * 1024 * 1024

ENTRY_BRANCH_OFFSET = 0x03
LOGO_RANGE = (0x04, 0xA0)
TITLE_RANGE = (0xA0, 0xAC)
CODE_RANGE = (0xAC, 0xB0)
FIXED_VALUE_OFFSET = 0xB2
CHECKSUM_RANGE = (0xA0, 0xBD)
CHECKSUM_OFFSET = 0xBD


def header_complement(rom):
    """The check gbafix writes into 0xBD."""
    total = sum(rom[CHECKSUM_RANGE[0]:CHECKSUM_RANGE[1]])
    return (-(0x19 + total)) & 0xFF


def main():
    parser = argparse.ArgumentParser(description='Validate a GBA ROM header.')
    parser.add_argument('rom', help='path to the .gba file')
    parser.add_argument('--title', help='expected ROM title (ROMTITLE in the Makefile)')
    parser.add_argument('--code', help='expected game code (ROMCODE in the Makefile)')
    args = parser.parse_args()

    if not os.path.isfile(args.rom):
        sys.exit('error: {} does not exist'.format(args.rom))

    with open(args.rom, 'rb') as rom_file:
        rom = rom_file.read()

    errors = []
    size = len(rom)

    if size < HEADER_SIZE:
        sys.exit('error: {} is {} bytes, too small to contain a cartridge header'.format(args.rom, size))

    if size > MAX_ROM_SIZE:
        errors.append('ROM is {} bytes, over the 32MB cartridge limit'.format(size))

    if rom[ENTRY_BRANCH_OFFSET] != 0xEA:
        errors.append('entry point at 0x000 is not an ARM branch (byte 0x003 is 0x{:02X}, expected 0xEA)'
                      .format(rom[ENTRY_BRANCH_OFFSET]))

    if not any(rom[LOGO_RANGE[0]:LOGO_RANGE[1]]):
        errors.append('Nintendo logo region (0x004-0x09F) is blank; the ROM will not boot')

    if rom[FIXED_VALUE_OFFSET] != 0x96:
        errors.append('fixed value at 0x0B2 is 0x{:02X}, expected 0x96'.format(rom[FIXED_VALUE_OFFSET]))

    title = rom[TITLE_RANGE[0]:TITLE_RANGE[1]].decode('ascii', 'replace').rstrip('\x00 ')
    code = rom[CODE_RANGE[0]:CODE_RANGE[1]].decode('ascii', 'replace').rstrip('\x00 ')

    if args.title is not None and title != args.title:
        errors.append('title is {!r}, expected {!r} (did gbafix run?)'.format(title, args.title))

    if args.code is not None and code != args.code:
        errors.append('game code is {!r}, expected {!r}'.format(code, args.code))

    expected_checksum = header_complement(rom)

    if rom[CHECKSUM_OFFSET] != expected_checksum:
        errors.append('header complement check is 0x{:02X}, expected 0x{:02X}'
                      .format(rom[CHECKSUM_OFFSET], expected_checksum))

    print('rom:      {}'.format(args.rom))
    print('size:     {} bytes ({:.1f} KiB)'.format(size, size / 1024))
    print('title:    {!r}'.format(title))
    print('code:     {!r}'.format(code))
    print('checksum: 0x{:02X}'.format(rom[CHECKSUM_OFFSET]))

    if errors:
        print('\nFAILED:')

        for error in errors:
            print('  - {}'.format(error))

        return 1

    print('\nheader OK')
    return 0


if __name__ == '__main__':
    sys.exit(main())
