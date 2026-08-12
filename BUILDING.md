# Building Snowboard Rush

## Dependencies

- A C++17 compiler (GCC, Clang, or MSVC)
- [raylib](https://www.raylib.com/) 4.x or 5.x
- CMake 3.16+
- (Optional) OpenCV for webcam motion control

### On Debian/Ubuntu

```sh
sudo apt install build-essential cmake libx11-dev libgl1-mesa-dev

# Optional: webcam support
sudo apt install libopencv-dev
```

### On macOS

```sh
brew install cmake raylib

# Optional: webcam support
brew install opencv
```

### On Windows

Download [raylib](https://github.com/raysan5/raylib/releases) prebuilt release and CMake from <https://cmake.org/download>.

## Build with CMake (recommended)

```sh
cd SnowboardRush
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DRAYLIB_DIR=/path/to/raylib    # e.g. ~/raylib-5.5_linux_amd64
cmake --build build
./build/snowboard_rush
```

If raylib is installed system-wide and discoverable via `pkg-config`, you can omit `-DRAYLIB_DIR`. The build system tries `pkg-config` and common install locations automatically.

### Webcam support (optional)

By default, webcam motion control is **disabled** for privacy. To enable it:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_WEBCAM_CONTROL=ON \
      -DRAYLIB_DIR=/path/to/raylib
cmake --build build
```

If OpenCV isn't found, the build proceeds keyboard-only without error. Once built with webcam support, you can toggle it in the Options menu (off by default).

### Asset path

The CMake target defines `ASSETS_PATH` to the project's `assets/` directory and copies `assets/` next to the binary, so the game finds its models no matter where it is launched from.

## Manual compilation (single command)

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

## Platform-specific notes

### Linux

If using a system package for raylib, you may need to install development headers. The build outputs an AppImage in CI that can run on any Linux distro.

### macOS

Ensure your Xcode Command Line Tools are up to date:

```sh
xcode-select --install
```

### Windows

MinGW cross-compilation is supported via the cmake toolchain file in `cmake/toolchains/mingw-w64-x86_64.cmake`. The CI uses this for the Windows release build.

## Troubleshooting

**"raylib not found"** → Install raylib or set `-DRAYLIB_DIR=/path/to/raylib`

**CMake complains about C++ version** → Use a compiler that supports C++17: GCC 7+, Clang 5+, or MSVC 2017+

**OpenGL errors at runtime** → Install `libgl1-mesa-dev` (Linux) or update your GPU drivers

**Webcam detection fails** → OpenCV is optional. The game runs fine on keyboard alone. If you want webcam, install `libopencv-dev` and rebuild with `-DENABLE_WEBCAM_CONTROL=ON`
