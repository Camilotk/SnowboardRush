# 🏂 Snowboard Rush

> **Carve down an endless mountain. Dodge trees. Grab coins. Hit ramps. Crash spectacularly. Chase your high score.**

A fast, fun 3D snowboarding endless runner. Built in C++17 with raylib. No bloat. Just pure downhill adrenaline.

---

## 🎮 What's it like?

Drop into a procedurally-generated mountain and **ride as far as you can**. The longer you survive, the faster it gets. Weave between trees and rocks, launch off ramps, grab coins for points, and try not to explode into a snowbank.

### The gameplay loop
1. **Steer left/right** to dodge obstacles
2. **Jump over low rocks** (high trees = instant crash)
3. **Hit ramps** for air time and bonus points
4. **Collect coins** to boost your score
5. **Speed increases constantly** — survive or restart and chase a better high score

### Features

✨ **Smooth, responsive controls**  
Lean left or right — your board follows your input immediately, no lag.

🎬 **Silky-smooth camera**  
Third-person view that flows with you down the mountain. Never feels floaty or disconnected.

❄️ **Beautiful procedural snow**  
Falling snow particles + a world-space snow shader make the mountain feel alive. All generated at runtime — no baked textures.

🎵 **Procedurally generated sounds**  
Jump, crash, coin collect, ramp launch — every sound is synthesized. Ambient wind loop reacts to your speed.

🏔️ **Endless difficulty**  
Density and speed ramp up smoothly. Early runs are forgiving; late-game runs demand precision.

🎯 **High score chase**  
Distance + coins = points. Beat your personal best and compete with yourself.

🎥 **Optional: Control with your webcam** (Off by default)  
Lean left/right in front of your camera. Hop to jump. Adds a fun physicality to the run. Privacy-first: turn it on in Options only if you want it.

---

## 🚀 Get it

### Download pre-built

Grab the latest release: [github.com/Camilotk/SnowboardRush/releases](https://github.com/Camilotk/SnowboardRush/releases)

- **Linux**: `Snowboard_Rush-Linux-x86_64.AppImage` — just make it executable and run
- **Windows**: `Snowboard_Rush-Windows-x86_64.zip` — extract and run `snowboard_rush.exe`

### Build from source

See [BUILDING.md](BUILDING.md) for detailed instructions.

**Quick start on Linux/macOS:**

```sh
git clone https://github.com/Camilotk/SnowboardRush.git
cd SnowboardRush
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/snowboard_rush
```

---

## 🎮 Controls

| Action | Key |
|--------|-----|
| Steer left | `A` or `←` |
| Steer right | `D` or `→` |
| Jump | `Space` |
| Start game | `Enter` or `Space` (from menu) |
| Pause | `Esc` |
| Restart (after crash) | `R` |

**Webcam (when enabled in Options):**
- Lean left/right to steer
- Small hop to jump
- Toggle with `C` or via Options menu

---

## 🛠️ Made with

- **raylib** — Fast, lightweight 3D graphics
- **C++17** — Modern, efficient
- **CMake** — Cross-platform builds (Linux, macOS, Windows)
- **OpenCV** (optional) — Webcam motion tracking

All 3D models and textures are hand-authored or procedurally generated. No heavy asset pipelines. Ships at ~15 MB.

---

## 📚 More details

- **[TECHNICAL.md](TECHNICAL.md)** — How the game works: rendering, collision, difficulty, webcam system
- **[BUILDING.md](BUILDING.md)** — Build instructions for all platforms
- **[LICENSE](LICENSE)** — GNU GPL v3

---

## 📝 License

Snowboard Rush is free software: GPL-3.0-or-later

Copyright © 2026 Camilo Cunha de Azevedo

Wind audio: CC-BY 3.0 (see [assets/audio/CREDITS.txt](assets/audio/CREDITS.txt))

---

## 🤔 FAQ

**Is webcam support required?**  
Nope! Webcam is optional and OFF by default. Full game works with just keyboard.

**Can I play on a low-end PC?**  
Yep. Raylib is lightweight and the game targets 60 fps on modest hardware.

**Can I mod or extend it?**  
Yes — it's GPL-licensed. Fork it, modify constants in `src/Constants.h`, tweak difficulty, add new obstacles, whatever.

**How high can scores go?**  
Theoretically forever, but most players tap out around 500–2000 meters. There's no built-in ceiling.

---

**Enjoy the run! 🏂**
