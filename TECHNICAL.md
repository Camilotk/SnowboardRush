# Technical Details

## Architecture

Snowboard Rush is built from scratch with **C++17** and **raylib 5.5**.

Every asset — 3D models, materials, textures, and sound effects — is generated procedurally or authored as plain-text Wavefront OBJ/MTL, except the ambient wind loop (CC-BY 3.0; see [`assets/audio/CREDITS.txt`](assets/audio/CREDITS.txt)). No external art downloads at build time.

### Project structure

```text
SnowboardRush/
├── CMakeLists.txt
├── README.md                   fun & gameplay-focused
├── TECHNICAL.md                this file
├── BUILDING.md                 build instructions
├── assets/
│   └── models/
│       ├── snowboarder.obj     low-poly rider (5 materials)
│       ├── snowboard.obj       the board
│       ├── pine_tree.obj       trees (foliage + trunk)
│       ├── rock.obj            jumpable rocks
│       ├── ramp.obj            launch ramps
│       └── coin.obj            spinning collectibles
└── src/
    ├── main.cpp                entry point
    ├── Constants.h             shared tuning values
    ├── Game.h / .cpp           state machine, loop, rendering, UI, collisions
    ├── Player.h / .cpp         steering, jump, gravity, crash
    ├── World.h / .cpp          endless course, spawning, recycling, difficulty
    ├── Obstacle.h / .cpp       obstacle types + collision config
    ├── Collectible.h           coin entity
    ├── Ramp.h                  ramp entity
    ├── CameraController.h/.cpp smooth third-person follow cam
    ├── SnowParticles.h/.cpp    ambient falling snow
    ├── SnowShader.h/.cpp       procedural world-space snow
    ├── SoundManager.h/.cpp     procedural sound effects
    └── WebcamInput.h/.cpp      optional webcam motion control (steer/jump)
```

## Game systems

### Endless level
The player stays near `z = 0`; the whole world scrolls toward the camera. Entities are tracked by a `coursePos` distance coordinate and recycled once they fall behind the camera, so memory stays bounded even after hours of play.

### Collision
Circle-vs-circle collision in the x-z plane with a vertical "clear height" check, so low rocks can be jumped while trees/barriers cannot.

### Difficulty
Speed and obstacle density both ramp up with distance traveled. The course gets progressively denser and faster to keep the challenge rising.

### Procedural generation
- **Snow mounds** — soft obstacles that scrub speed without crashing
- **Ground texture** — Perlin noise generated and tiled
- **Mountains** — distant backdrop, procedural cone geometry
- **Audio** — wind loop and sound effects (jump, crash, coin, ramp)

### Webcam motion control (optional)
Built-in optional motion tracking via OpenCV (requires separate compilation flag):
- **Face detection** (Haar cascade) for absolute position tracking
- **Motion blob fallback** for when face isn't visible
- **One Euro filter** for smooth, responsive steering without jitter
- **Automatic neutral calibration** to handle camera bias
- **Robust recovery** — reopens camera if it disconnects mid-session

## Performance notes

- **Rendering** — flat-shaded OBJ models, raylib immediate-mode API
- **Camera** — third-person smooth follow with spring physics
- **Collision** — simple circle detection, O(n) sweep
- **Memory** — bounded by max concurrent entities (obstacles, coins, ramps)
- **FPS** — targets 60 fps, vsync enabled

## License

GNU General Public License v3 or later (GPL-3.0-or-later).

See [LICENSE](LICENSE) for full text.
