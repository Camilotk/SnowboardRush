#pragma once

#include "raylib.h"

namespace sb {

struct Collectible {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float coursePos = 0.0f;
    float height = 0.0f;     // float height above the slope surface
    float radius = 0.55f;
    float spin = 0.0f;       // rotation around Y
    float bobPhase = 0.0f;
    bool active = false;
};

} // namespace sb
