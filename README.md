# GBA_Game

A 2D puzzle game for the Game Boy Advance.

[![CI](https://github.com/JacobPoteet/GBA_Game/actions/workflows/ci.yml/badge.svg)](https://github.com/JacobPoteet/GBA_Game/actions/workflows/ci.yml)

> **Status: project skeleton.** The toolchain, project layout and CI/CD pipeline are in place and
> produce a bootable ROM. There are no puzzle rules yet — what runs today is a title screen and a
> grid you can move a cursor around, which exists to prove the whole pipeline works end to end.

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
docs/                Architecture and day-to-day development notes
graphics/            Sprite sheets: 4bpp .bmp plus a .json descriptor per asset
audio/               Maxmod modules and samples (empty for now)
include/             Headers, all prefixed gp_
src/                 Implementation, all prefixed gp_ (except main.cpp)
tests/               Host unit tests and their CMake project
tools/               verify_rom.py (ROM header check), gen_placeholder_art.py
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

| Button | Action |
| --- | --- |
| D-pad | Move the cursor |
| A | Cycle the tile under the cursor |
| START | Shuffle the board (title screen: start) |
| B | Back to the title screen |

## CI/CD

Every push and pull request runs three jobs in parallel:

* **Build ROM** — compiles inside `devkitpro/devkitarm`, then runs `tools/verify_rom.py` to check
  the cartridge header (entry branch, Nintendo logo, title, game code, complement checksum). A ROM
  that builds but would not boot fails the build. The `.gba` is uploaded as an artifact.
* **Host unit tests** — CMake + CTest over the engine-free game logic.
* **clang-format** — formatting is enforced, not suggested.

Pushing a `v*` tag builds the ROM again, verifies it, and publishes it to a GitHub Release:

```bash
git tag v0.1.0 && git push origin v0.1.0
```

## License

MIT — see [LICENSE](LICENSE). Butano is licensed separately under the zlib license.
