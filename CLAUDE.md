# CLAUDE.md

Guidance for Claude Code working in this repository.

## What this is

A 2D puzzle game for the Game Boy Advance, built with **Butano** (C++20) on **devkitARM**.
Currently a skeleton: the pipeline produces a bootable ROM, but there are no puzzle rules yet.

## Build and test

```bash
make -j4                                          # builds gba_game.gba (needs devkitARM)
cmake -S tests -B build-host                      # host unit tests, no devkitARM needed
cmake --build build-host
ctest --test-dir build-host --output-on-failure
clang-format --dry-run --Werror src/*.cpp include/*.h tests/*.cpp tests/*.h
```

`make` only works inside devkitPro's MSYS2 shell on Windows. If no ARM toolchain is available,
work on the pure logic and let CI build the ROM — do not claim the ROM builds without evidence.

## Non-obvious constraints

These will bite silently if ignored:

* **Source file names must be unique across the entire project.** Butano's makefile flattens every
  source directory onto one `VPATH`. Everything in `src/` and `include/` is prefixed `gp_` for this
  reason. Keep the prefix on new files.
* **Files that include any `bn_` header cannot be unit tested.** Keep puzzle logic free of Butano
  so `tests/` can compile it on the host. This split is the point of the architecture — see
  `docs/architecture.md`.
* **Graphics must be 4bpp BMPs with a 16-colour palette and a 40-byte BITMAPINFOHEADER.** Butano's
  `bmp.py` rejects V4/V5 headers, which is what most image editors emit by default. Every `.bmp`
  in `graphics/` needs a matching `.json`. Regenerate placeholders with
  `python tools/gen_placeholder_art.py`.
* **Frame order in `graphics/gp_tile.bmp` must match `gp::tile_kind`.** The mapping lives in
  `tile_kind_graphics_index()` and in `TILE_COLORS` in the art generator. A test covers the round
  trip, but the art side is not checked automatically.
* **Butano is a pinned submodule** at `third_party/butano`. Do not edit anything inside it.
  Clones need `--recurse-submodules`.

## Style

Butano's conventions, enforced by `.clang-format` (pinned to clang-format 20.1.7):
Allman braces, 4 spaces, 120 columns, `_leading_underscore` for private members, `snake_case`
types. Run `clang-format -i` before committing; CI fails on formatting.

## Adding things

* **Source file** — drop it in `src/`, prefix `gp_`, no build file edit needed. If it is
  Butano-free, add it to `tests/CMakeLists.txt` too.
* **Test** — new `tests/gp_*_test.cpp`, list it in `tests/CMakeLists.txt`, use `GP_TEST` /
  `GP_CHECK` / `GP_CHECK_EQ`. Tests self-register.
* **Scene** — subclass `gp::scene`, add a `gp::scene_type` enumerator, wire it into `create_scene`
  in `src/main.cpp`.

## CI

`.github/workflows/ci.yml` runs three jobs on every push and PR: ROM build inside the
`devkitpro/devkitarm` container followed by `tools/verify_rom.py` header validation, host unit
tests, and a clang-format check. Tagging `v*` triggers `release.yml`, which publishes the ROM to a
GitHub Release.
