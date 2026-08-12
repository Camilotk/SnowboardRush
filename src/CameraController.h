#pragma once

#include "raylib.h"

namespace sb {

class CameraController {
public:
    void Reset();
    void Update(float dt, const Vector3& playerPos, float speed);

    Camera3D camera = {};

private:
    float smoothX_ = 0.0f;
    float smoothY_ = 0.0f;
};

} // namespace sb
