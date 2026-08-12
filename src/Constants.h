// Snowboard Rush - a downhill snowboarding endless runner.
// Copyright (C) 2026 Camilo Cunha de Azevedo
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

// Resolved at compile time by CMake (absolute path to assets/), with a
// sensible fallback for manual compilation from the project root.
#ifndef ASSETS_PATH
#define ASSETS_PATH "assets/"
#endif

namespace sb {

// --- course ---
constexpr float COURSE_HALF_WIDTH = 10.0f;  // playable x range [-10, 10]
constexpr float COURSE_EDGE_X = COURSE_HALF_WIDTH - 0.5f; // clamp line / boundary marker x
constexpr float PLAYER_Z = 0.0f;            // player is fixed at z = 0

// --- player ---
constexpr float PLAYER_REST_Y = 0.32f;      // snowboard ride height above snow
constexpr float PLAYER_RADIUS = 0.55f;      // collision radius (x-z circle)
constexpr float STEER_ACCEL = 55.0f;        // horizontal acceleration
constexpr float STEER_DAMP = 8.0f;          // horizontal damping (release glide)
constexpr float MAX_STEER_SPEED = 13.0f;    // horizontal speed cap
constexpr float MAX_LEAN = 0.30f;           // max bank angle (radians)
constexpr float GRAVITY = -32.0f;           // world gravity
constexpr float JUMP_SPEED = 13.5f;         // manual jump impulse
constexpr float RAMP_LAUNCH_FACTOR = 0.45f; // ramp vy = factor * downhill speed
constexpr float LANDING_SCRUB = 0.01f;      // speed loss per unit landing impact

// --- downhill / speed ---
constexpr float SLOPE = 0.40f;              // downhill grade (rise over run)
constexpr float SLOPE_ANGLE = 0.3805f;      // atan(SLOPE), precomputed
constexpr float SLOPE_SIN = 0.3714f;        // sin(atan(SLOPE)) - downhill gravity component
constexpr float SLOPE_COS = 0.9285f;        // cos(atan(SLOPE)) - surface normal component
constexpr float PLAYER_FORWARD_PITCH = 0.38f; // constant nose-down lean (matches grade)

constexpr float BASE_SPEED = 14.0f;         // starting downhill speed
constexpr float MAX_SPEED = 52.0f;          // maximum downhill speed
constexpr float SNOW_MU = 0.04f;            // kinetic friction (groomed snow)
constexpr float DRAG_K = 0.0043f;           // quadratic air drag (terminal ~50 u/s)
constexpr float CARVE_SCRUB = 0.02f;        // speed loss per unit steer input
constexpr float SNOW_SCRUB = 8.0f;          // speed loss per second deep in a snow mound
constexpr float RAMP_LANDING_CLEAR = 60.0f; // obstacle-free distance after a ramp

// --- speed-feel intensity (knobs for a future sensory settings screen) ---
// Scale the strength of each speed sensation effect; 1.0 is the default.
// A "Calm" profile would lower these, a "Boost" profile could raise them.
constexpr float SPEED_FX_FOV = 1.0f;    // FOV widening at speed
constexpr float SPEED_FX_WIND = 1.0f;   // wind audio volume
constexpr float SPEED_FX_GROUND = 1.0f; // groomed reference-pattern contrast

// --- spawning / recycling ---
constexpr float SPAWN_DIST = 120.0f;        // fill content up to this far ahead
constexpr float DESPAWN_Z = 24.0f;          // recycle once behind this z
constexpr float MARKER_SPACING = 8.0f;      // distance between boundary flag poles

// --- scoring ---
constexpr int COIN_SCORE = 50;
constexpr int RAMP_SCORE = 100;
constexpr int POINTS_PER_METER = 2;

// --- game state ---
enum class GameState { Menu, Playing, Paused, Options, Crashed };

} // namespace sb
