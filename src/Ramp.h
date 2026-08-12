#pragma once

#include "raylib.h"

namespace sb {

struct Ramp {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float coursePos = 0.0f;
    float halfWidth = 2.0f;   // x extent of the ramp surface
    float length = 8.0f;      // z extent (used for trigger zone)
    bool used = false;        // has the player already launched from it
    bool active = false;
};

} // namespace sb
