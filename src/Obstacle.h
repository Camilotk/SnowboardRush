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

#include "raylib.h"

namespace sb {

enum class ObstacleType { Tree, Rock, Barrier };

struct Obstacle {
    ObstacleType type = ObstacleType::Tree;
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float radius = 0.6f;      // collision radius (x-z)
    float clearHeight = 1.0f; // player must be above this y to pass safely
    float coursePos = 0.0f;   // spawn/recycle tracking coordinate
    float yaw = 0.0f;         // cosmetic rotation
    bool active = false;
};

// Per-type collision configuration.
float ObstacleRadius(ObstacleType type);
float ObstacleClearHeight(ObstacleType type);

} // namespace sb
