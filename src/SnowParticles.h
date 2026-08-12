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
