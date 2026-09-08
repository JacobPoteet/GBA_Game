# GBA_Game

A 2D puzzle game for the Game Boy Advance.

[![CI](https://github.com/JacobPoteet/GBA_Game/actions/workflows/ci.yml/badge.svg)](https://github.com/JacobPoteet/GBA_Game/actions/workflows/ci.yml)

> **Status: the game runs, with no picture yet.** The whole simulation lives in `src/` with no
> Butano in it, and the host tests play it: walking, connecting, matching, mismatching, chases and
> level completion. Levels are authored in Tiled and CI fails the build if one cannot be finished.
> Phase 3 gave it sprites, a camera, a background and audible chirps, so level 1 is playable end to
> end. Story, progression and real art are next.

Working title **Chirp**. You are looking for the person who matches you, and you cannot find them
until you have paired off everyone else. The design, the decision log and the roadmap live in the
project wiki.

## Tech stack

| Piece | Choice | Why |
| --- | --- | --- |
| Engine | [Butano](https://github.com/GValiente/butano) (C++20) | The most complete 2D GBA engine: sprites, backgrounds, text and audio behind a modern RAII API, plus an asset pipeline that compiles art into the ROM. |
| Toolchain | [devkitARM](https://devkitpro.org/) | What Butano targets. Supplies `arm-none-eabi-gcc`, `grit`, `mmutil` and `gbafix`. |
| ROM build | GNU Make | Butano ships its own `butano.mak`; the top level `Makefile` just configures it. |
| Test build | CMake + CTest | Host compiler, used only for the engine-free game logic. |
| CI/CD | GitHub Actions | Builds the ROM in the official `devkitpro/devkitarm` container, runs the host tests, checks formatting, and publishes tagged releases. |
| Formatting | clang-format (pinned) | Version pinned in CI so the formatter never fails a build on its own schedule. |

## Layout

```
.github/workflows/   CI (build + test + format) and tagged releases
maps/                Tiled .tmx levels plus the placeholder tileset
graphics/            Sprite sheets: 4bpp .bmp plus a .json descriptor per asset
audio/               One base chirp sample per clan, 8 bit mono
include/             Headers, all prefixed gp_
src/                 Implementation, all prefixed gp_ (except main.cpp)
tests/               Host unit tests, the level solver, and their CMake project
tools/               gp_import_maps.py (Tiled to C++), art and audio generators, verify_rom.py
third_party/butano/  Butano, pinned as a git submodule
Makefile             Builds gba_game.gba
```

The one structural rule worth knowing: **source file names must be globally unique**, because
Butano's makefile flattens every source directory onto a single `VPATH`. Hence the `gp_` prefix.

## Getting started

Clone with submodules — the build will not work without Butano:

```bash
git clone --recurse-submodules https://github.com/JacobPoteet/GBA_Game.git
```

Already cloned without them?

```bash
git submodule update --init --recursive
```

### Install devkitARM

* **Windows** — run the [devkitPro installer](https://github.com/devkitPro/installer/releases) and
  tick the **GBA Development** group. Build from the *MSYS2* shell it installs, not from PowerShell.
* **macOS / Linux** — follow the [devkitPro pacman instructions](https://devkitpro.org/wiki/Getting_Started),
  then `sudo dkp-pacman -S gba-dev`.

### Build the ROM

```bash
make -j4
```

That produces `gba_game.gba`. Open it in [mGBA](https://mgba.io/) to play.

### Run the logic tests

These need only a host C++20 compiler and CMake — no devkitARM:

```bash
cmake -S tests -B build-host && cmake --build build-host && ctest --test-dir build-host --output-on-failure
```

## Controls

The rule is **A commits, B costs nothing**. Every irreversible act is A; B is only ever information
or movement. D-pad walks: tapping a direction turns you, holding it walks.

| Situation | A | B (tap) | B (hold) |
| --- | --- | --- | --- |
| Facing a stranger, empty-handed | Connect | They chirp | Run |
| Facing a stranger, follower in tow | Attempt match | They chirp | Run |
| Turned around, facing your follower | Release them | Follower chirps | Run |
| Nothing in front of you | — | — | Run |

## CI/CD

Every push and pull request runs three jobs in parallel:

* **Build ROM** — compiles inside `devkitpro/devkitarm`, then runs `tools/verify_rom.py` to check
  the cartridge header (entry branch, Nintendo logo, title, game code, complement checksum). A ROM
  that builds but would not boot fails the build. The `.gba` is uploaded as an artifact.
* **Host unit tests** — CMake + CTest over the engine-free simulation.
* **clang-format** — formatting is enforced, not suggested.

Pushing a `v*` tag builds the ROM again, verifies it, and publishes it to a GitHub Release:

```bash
git tag v0.1.0 && git push origin v0.1.0
```

## License

MIT — see [LICENSE](LICENSE). Butano is licensed separately under the zlib license.
