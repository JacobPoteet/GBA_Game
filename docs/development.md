# Development

## Toolchain setup

### Windows

1. Install [devkitPro](https://github.com/devkitPro/installer/releases) and select the
   **GBA Development** component group.
2. Build from the **MSYS2** shell the installer sets up (Start menu, "MSYS2"). PowerShell and
   `cmd` do not have `make`, `grit` or the `DEVKITARM` environment variable.
3. `cd` to the repo and run `make -j4`.

If the asset step cannot find Python, pass the interpreter explicitly:

```bash
make PYTHON=python
```

The `Makefile` auto-detects `python3` and falls back to `python`; this override exists for
environments where neither name resolves to the interpreter you want.

### macOS / Linux

```bash
sudo dkp-pacman -S gba-dev
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
make -j4
```

### No toolchain at all

You can still work on the puzzle logic and its tests with nothing but CMake and a C++20 compiler,
and let CI build the ROM. See "Host unit tests" below.

## Common tasks

| Task | Command |
| --- | --- |
| Build the ROM | `make -j4` |
| Clean | `make clean` |
| Host unit tests | `cmake -S tests -B build-host && cmake --build build-host && ctest --test-dir build-host --output-on-failure` |
| Check formatting | `clang-format --dry-run --Werror src/*.cpp include/*.h tests/*.cpp tests/*.h` |
| Fix formatting | `clang-format -i src/*.cpp include/*.h tests/*.cpp tests/*.h` |
| Verify a built ROM | `python tools/verify_rom.py gba_game.gba --title "GBA PUZZLE" --code PZLE` |
| Regenerate placeholder art | `python tools/gen_placeholder_art.py` |

Install the same formatter CI uses so results match:

```bash
pip install clang-format==20.1.7
```

## Running the ROM

[mGBA](https://mgba.io/) is the recommended emulator — accurate, and its logging integrates with
Butano's `BN_LOG`. Open `gba_game.gba` in it.

Butano writes log output through the emulator, so `BN_LOG("value: ", x)` shows up in mGBA's log
window. Asserts (`BN_ASSERT`) halt with the message on screen.

## Adding a sprite

1. Export a **4bpp BMP with a 16-colour palette** into `graphics/`. Most editors default to a
   32-bit or V5-header BMP, which Butano rejects. In GIMP: *Image > Mode > Indexed* (16 colours),
   then export as BMP with *Do not write colour space information* ticked.
2. Add `graphics/your_sprite.json`:
   ```json
   {
       "type": "sprite",
       "height": 16
   }
   ```
   `height` is one frame's height; a taller image becomes multiple frames.
3. Rebuild. Include the generated header and use the item:
   ```cpp
   #include "bn_sprite_items_your_sprite.h"

   bn::sprite_ptr sprite = bn::sprite_items::your_sprite.create_sprite(0, 0);
   ```

Palette index 0 is transparent. Sprite dimensions must be a valid GBA size (8, 16, 32 or 64 on
each axis).

## Adding a source file

Put it in `src/` (headers in `include/`) and prefix the name `gp_`. No build file changes are
needed — the `Makefile` globs `src/`. If it is pure logic with no `bn_` includes, also add it to
`tests/CMakeLists.txt` so it can be unit tested.

Butano's makefile flattens all source directories into one `VPATH`, so **file names must be
unique across the whole project**. That is the reason for the prefix.

## Adding a test

Create `tests/gp_something_test.cpp`, list it in `tests/CMakeLists.txt`, and write:

```cpp
#include "gp_test_framework.h"
#include "gp_board.h"

GP_TEST(descriptive_name_of_the_behaviour)
{
    gp::board board;
    GP_CHECK(!board.full());
    GP_CHECK_EQ(board.count(gp::tile_kind::empty), gp::board::cell_count);
}
```

Tests self-register; there is no list to update in `gp_tests_main.cpp`.

## Updating Butano

```bash
git -C third_party/butano fetch --depth 1 origin master
git -C third_party/butano checkout FETCH_HEAD
git add third_party/butano
git commit -m "deps: bump butano"
```

CI builds against the pinned commit, so a bump is a reviewable change like any other.

## Releasing

```bash
git tag v0.1.0
git push origin v0.1.0
```

`.github/workflows/release.yml` builds the ROM, verifies its header, and attaches
`gba_game-v0.1.0.gba` to a generated GitHub Release.
