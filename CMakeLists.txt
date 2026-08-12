cmake_minimum_required(VERSION 3.16)
project(SnowboardRush LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------------------------------------------------------------------------
# Locate raylib.
#
# Preferred: pass -DRAYLIB_DIR=/path/to/raylib  (a directory containing
#            include/raylib.h and lib/libraylib.*).
#
# Fallback:  pkg-config (raylib), then common system locations.
# ---------------------------------------------------------------------------
set(RAYLIB_DIR "" CACHE PATH "Path to a raylib install (contains include/ and lib/)")

if(RAYLIB_DIR)
    set(RAYLIB_INCLUDE_DIR ${RAYLIB_DIR}/include)
    find_library(RAYLIB_LIBRARY NAMES raylib
        PATHS ${RAYLIB_DIR}/lib ${RAYLIB_DIR}/lib64 NO_DEFAULT_PATH)
    if(NOT RAYLIB_LIBRARY)
        # Explicit static fallback for the official GitHub release layout.
        if(EXISTS ${RAYLIB_DIR}/lib/libraylib.a)
            set(RAYLIB_LIBRARY ${RAYLIB_DIR}/lib/libraylib.a)
        endif()
    endif()
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(RAYLIB QUIET raylib)
    endif()

    if(RAYLIB_FOUND)
        set(RAYLIB_INCLUDE_DIR ${RAYLIB_INCLUDE_DIRS})
        set(RAYLIB_LIBRARY ${RAYLIB_LIBRARIES})
    else()
        find_path(RAYLIB_INCLUDE_DIR raylib.h
            PATHS /usr/local/include /usr/include)
        find_library(RAYLIB_LIBRARY NAMES raylib
            PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu)
    endif()
endif()

if(NOT RAYLIB_INCLUDE_DIR OR NOT RAYLIB_LIBRARY)
    message(FATAL_ERROR
        "raylib not found. Install raylib, or configure with:\n"
        "    cmake -DRAYLIB_DIR=/path/to/raylib ..")
endif()

message(STATUS "raylib include: ${RAYLIB_INCLUDE_DIR}")
message(STATUS "raylib library: ${RAYLIB_LIBRARY}")

# ---------------------------------------------------------------------------
# Target
# ---------------------------------------------------------------------------
add_executable(snowboard_rush
    src/main.cpp
    src/Game.cpp
    src/Player.cpp
    src/World.cpp
    src/Obstacle.cpp
    src/CameraController.cpp
    src/SnowParticles.cpp
    src/SoundManager.cpp
)

target_include_directories(snowboard_rush PRIVATE src ${RAYLIB_INCLUDE_DIR})

# Absolute path to the assets directory so the game can find its models when
# launched from any working directory.
target_compile_definitions(snowboard_rush PRIVATE
    ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")

target_link_libraries(snowboard_rush PRIVATE ${RAYLIB_LIBRARY})

if(UNIX AND NOT APPLE)
    # raylib's desktop backend (GLFW/X11) and standard POSIX dependencies.
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    target_link_libraries(snowboard_rush PRIVATE
        OpenGL::GL X11::X11 m pthread dl rt)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(snowboard_rush PRIVATE
        ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY}
        "-framework OpenGL")
endif()

# Convenience: copy assets next to the binary so it also runs without the
# compile-time ASSETS_PATH (e.g. after copying the build output elsewhere).
add_custom_command(TARGET snowboard_rush POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:snowboard_rush>/assets)
