# CLAUDE.md

Guidance for Claude Code working in this repository.

## What this is

A 2D top-down puzzle game for the Game Boy Advance, built with **Butano** (C++20) on
**devkitARM**. Working title *Chirp*: you pair strangers off by matching what they look like
against what they sound like, and you cannot find your own match until everyone else has theirs.

The rules all live in `gp::level_state` and are covered by host tests. Levels come from `maps/*.tmx`
through `tools/gp_import_maps.py`. Sprites, sound and a camera are not written yet.

**Documentation lives in the wiki, not here.** The design, the architecture, the roadmap and the
decision log are all notes there. This repo keeps `README.md` and this file and nothing else that
would duplicate one. Check the wiki before writing a doc, and update the note covering whatever you
changed in the same pass.

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
  the wiki's *The Pure Split*.
* **Graphics must be 4bpp BMPs with a 16-colour palette and a 40-byte BITMAPINFOHEADER.** Butano's
  `bmp.py` rejects V4/V5 headers, which is what most image editors emit by default. Every `.bmp`
  in `graphics/` needs a matching `.json`. `graphics/` is empty until phase 3.
* **`gp::level_state` never draws and never plays a sound.** It queues a `level_event` and the
  scene drains the queue. Calling into Butano from it would make the whole simulation untestable.
* **The solver lives in `tests/`, not `src/`.** Butano globs `src/*.cpp` into the ROM, and the game
  has no use for it at runtime.
* **`generated/` is build output, never edited or committed.** `tools/gp_import_maps.py` writes it
  from `maps/*.tmx`, driven by `EXTTOOL` for the ROM and by a custom command for the host tests.
  Edit the `.tmx` in Tiled instead.
* **Tiled layers must use CSV encoding.** The importer refuses base64, and the message says so.
  A map needs a `collision` tile layer, an `entities` object layer with one object named `player`,
  and named polylines on `zones` for flee routes.
* **Butano is a pinned submodule** at `third_party/butano`. Do not edit anything inside it.
  Clones need `--recurse-submodules`.

## Style

Butano's conventions, enforced by `.clang-format` (pinned to clang-format 20.1.7):
Allman braces, 4 spaces, 120 columns, `_leading_underscore` for private members, `snake_case`
types. Run `clang-format -i` before committing; CI fails on formatting.

## Adding things

* **Source file** — drop it in `src/`, prefix `gp_`, no build file edit needed. If it is
  Butano-free, add it to `tests/CMakeLists.txt` too, or the host tests will not cover it.
* **Test** — new `tests/gp_*_test.cpp`, list it in `tests/CMakeLists.txt`, use `GP_TEST` /
  `GP_CHECK` / `GP_CHECK_EQ`. Tests self-register.
* **Scene** — subclass `gp::scene`, add a `gp::scene_type` enumerator, wire it into `create_scene`
  in `src/main.cpp`.

## CI

`.github/workflows/ci.yml` runs three jobs on every push and PR: ROM build inside the
`devkitpro/devkitarm` container followed by `tools/verify_rom.py` header validation, host unit
tests, and a clang-format check. Tagging `v*` triggers `release.yml`, which publishes the ROM to a
GitHub Release.
