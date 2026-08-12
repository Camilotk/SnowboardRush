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
