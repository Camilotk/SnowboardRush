#pragma once

#include "raylib.h"
#include "Constants.h"

namespace sb {

class Player {
public:
    Player();

    void Reset();
    void Update(float dt, float steerInput, float speed);
    void Jump();
    void LaunchFromRamp(float power);
    void Crash();

    bool IsAirborne() const { return !onGround; }

    Vector3 position;   // (x, y, z=0); y is ride height / airborne height
    float vx = 0.0f;    // horizontal velocity
    float vy = 0.0f;    // vertical velocity
    bool onGround = true;
    bool crashed = false;

    float lean = 0.0f;       // roll around z (steering lean)
    float pitch = 0.0f;      // rotation around x (jump tilt)
    float crashSpin = 0.0f;  // tumble angle while crashed
    float landingImpact = 0.0f; // downward speed at last touchdown (0 while grounded)
};

} // namespace sb
