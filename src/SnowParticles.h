#pragma once

#include "raylib.h"

#include <random>
#include <vector>

namespace sb {

struct Snowflake {
    Vector3 pos = { 0.0f, 0.0f, 0.0f };
    Vector3 vel = { 0.0f, 0.0f, 0.0f };
    float size = 0.03f;
};

class SnowParticles {
public:
    explicit SnowParticles(int count);
    void Reset();
    void Update(float dt);
    void Draw();

private:
    void Respawn(Snowflake& f);

    std::vector<Snowflake> flakes_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist01_;
};

} // namespace sb
