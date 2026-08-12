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
