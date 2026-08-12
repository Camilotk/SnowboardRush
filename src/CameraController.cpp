#include "CameraController.h"
#include "Constants.h"

#include <algorithm>

namespace sb {

namespace {
constexpr float kCamDistZ = 9.5f;   // distance behind the player
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
    // camera sits on that raised surface and looks down the mountain.
    camera.position.x = smoothX_;
    camera.position.y = SLOPE * kCamDistZ + kCamHeight + smoothY_;
    camera.position.z = kCamDistZ;

    camera.target.x = smoothX_;
    camera.target.y = playerPos.y + 0.7f;
    camera.target.z = 0.0f;

    // Widening FOV at higher speeds sells the acceleration.
    const float speedT = std::min(1.0f, speed / MAX_SPEED);
    camera.fovy = 62.0f + speedT * 13.0f;
}

} // namespace sb
