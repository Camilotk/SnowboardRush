# Snowboard Rush

A polished, low-poly 3D snowboard endless runner built from scratch with
**C++17** and **raylib**. Ride downhill, steer between trees and rocks, hit
ramps, collect coins, and chase your high score as the mountain keeps getting
faster.

Every asset — 3D models, materials, textures, and sound effects — is generated
procedurally or authored as plain-text Wavefront OBJ/MTL, except the ambient
wind loop, which is a bundled CC-BY 3.0 recording (see
[`assets/audio/CREDITS.txt`](assets/audio/CREDITS.txt)). There are no external
art, model, or audio downloads at build time.

## Dependencies

- A C++17 compiler (GCC, Clang, or MSVC)
- [raylib](https://www.raylib.com/) 4.x or 5.x
- CMake 3.16+

On Debian/Ubuntu you can install the dependencies with:

```sh
sudo apt install build-essential cmake libx11-dev libgl1-mesa-dev
```

raylib itself can be installed via your package manager (if available), built
from source, or downloaded as a prebuilt release from
<https://github.com/raysan5/raylib/releases>.

## Build

### With CMake (recommended)

```sh
cd SnowboardRush
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DRAYLIB_DIR=/path/to/raylib    # e.g. ~/raylib-5.5_linux_amd64
cmake --build build
./build/snowboard_rush
```

If raylib is installed system-wide and discoverable via `pkg-config`, you can
omit `-DRAYLIB_DIR`. The build system tries `pkg-config` and common install
locations automatically.

The CMake target defines `ASSETS_PATH` to the project's `assets/` directory and
also copies `assets/` next to the binary, so the game finds its models no
matter where it is launched from.

### Manual compilation (single command)

From the project root:

```sh
g++ -std=c++17 -O2 -Isrc -I/path/to/raylib/include \
    -DASSETS_PATH='"assets/"' \
    src/*.cpp /path/to/raylib/lib/libraylib.a \
    -o snowboard_rush -lm -lpthread -ldl
```

> On macOS link against the raylib framework/`.a` and add
> `-framework Cocoa -framework IOKit -framework OpenGL`. On Windows, link
> `raylib.lib` and the Win32/OpenGL system libraries.

## Controls

| Key                          | Action                  |
|------------------------------|-------------------------|
| `A` / `D` or `←` / `→`       | Steer left / right      |
| `Space`                      | Jump                    |
| `R`                          | Restart (after crash)   |
| `Enter` / `Space`            | Start (from menu)       |
| `Esc`                        | Quit                    |

## Game objective

Ride as far as possible down the mountain. Score points by covering distance
and collecting gold coins. Ramps launch you into the air (bonus points) and let
you hop over low rocks. Hitting a tree, rock, or barrier ends the run — the
course speeds up and grows denser the longer you survive.

## Project structure

```text
SnowboardRush/
├── CMakeLists.txt
├── README.md
├── assets/
│   └── models/
│       ├── snowboarder.obj / .mtl   low-poly rider (5 materials)
│       ├── snowboard.obj  / .mtl    the board
│       ├── pine_tree.obj  / .mtl    trees (foliage + trunk)
│       ├── rock.obj       / .mtl    jumpable rocks
│       ├── ramp.obj       / .mtl    launch ramps
│       └── coin.obj       / .mtl    spinning collectibles
└── src/
    ├── main.cpp            entry point
    ├── Constants.h         shared tuning values
    ├── Game.h / .cpp       state machine, loop, rendering, UI, collisions
    ├── Player.h / .cpp     steering, jump, gravity, crash
    ├── World.h / .cpp      endless course, spawning, recycling, difficulty
    ├── Obstacle.h / .cpp   obstacle types + collision config
    ├── Collectible.h       coin entity
    ├── Ramp.h              ramp entity
    ├── CameraController.h/.cpp  smooth third-person follow cam
    ├── SnowParticles.h/.cpp     ambient falling snow
    └── SoundManager.h/.cpp      procedural sound effects
```

## Implementation notes

- **Endless level** — the player stays near `z = 0`; the whole world scrolls
  toward the camera. Entities are tracked by a `coursePos` distance coordinate
  and recycled once they fall behind the camera, so memory stays bounded.
- **Collision** — circle-vs-circle in the x-z plane with a vertical
  "clear height" check, so low rocks can be jumped while trees/barriers cannot.
- **Difficulty** — speed and obstacle density both ramp up with distance.
- **Assets** — OBJ files are flat-shaded with per-face normals; MTL `Kd`
  colors are read directly by raylib. Ground, snow mounds, mountains, barriers,
  and audio are generated procedurally at startup.

## License

Snowboard Rush is free software licensed under the GNU General Public License,
version 3 or (at your option) any later version.

```
SPDX-License-Identifier: GPL-3.0-or-later
```

Copyright (C) 2026 Camilo Cunha de Azevedo.

See [LICENSE](LICENSE) for the full license text, or
<https://www.gnu.org/licenses/gpl-3.0.html>.

The bundled wind loop (`assets/audio/wind.ogg`) is licensed separately under
CC-BY 3.0; attribution is recorded in [`assets/audio/CREDITS.txt`](assets/audio/CREDITS.txt).
