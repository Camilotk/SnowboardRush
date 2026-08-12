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

#include "CameraController.h"
#include "Constants.h"

#include <algorithm>

namespace sb {

namespace {
constexpr float kCamDistZ = 9.5f;   // base distance behind the player
constexpr float kCamHeight = 2.1f;  // height above the slope surface
} // namespace

void CameraController::Reset() {
    smoothX_ = 0.0f;
    smoothY_ = 0.0f;

    camera.position = { 0.0f, 4.5f, kCamDistZ };
    camera.target = { 0.0f, 1.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 62.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void CameraController::Update(float dt, const Vector3& playerPos, float speed) {
    // Exponential smoothing: framerate-independent, no jitter.
    const float k = std::min(1.0f, 12.0f * dt);
    smoothX_ += (playerPos.x - smoothX_) * k;
    smoothY_ += (playerPos.y * 0.3f - smoothY_) * k;   // weak vertical follow

    // Ride along the slope: behind the player the ground is higher, so the
    // camera sits on that raised surface and looks down the mountain. The
    // camera backs off at speed to widen the view and sell the velocity.
    const float speedT = std::min(1.0f, speed / MAX_SPEED);
    const float dist = kCamDistZ + speedT * 3.0f;
    camera.position.x = smoothX_;
    camera.position.y = SLOPE * dist + kCamHeight + smoothY_;
    camera.position.z = dist;

    camera.target.x = smoothX_;
    camera.target.y = playerPos.y + 0.7f;
    camera.target.z = 0.0f;

    // Widening FOV at higher speeds sells the acceleration.
    camera.fovy = 62.0f + speedT * (SPEED_FX_FOV * 20.0f);
}

} // namespace sb
