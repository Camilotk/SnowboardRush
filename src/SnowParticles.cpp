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

#include "SnowParticles.h"

namespace sb {

SnowParticles::SnowParticles(int count) : rng_(std::random_device{}()), dist01_(0.0f, 1.0f) {
    flakes_.resize(count);
    Reset();
}

void SnowParticles::Reset() {
    for (auto& f : flakes_) {
        Respawn(f);
        // Stagger so snow is already falling at start.
        f.pos.y = 1.0f + dist01_(rng_) * 11.0f;
    }
}

void SnowParticles::Respawn(Snowflake& f) {
    f.pos.x = -16.0f + dist01_(rng_) * 32.0f;
    f.pos.y = 10.0f + dist01_(rng_) * 4.0f;
    f.pos.z = -24.0f + dist01_(rng_) * 30.0f;
    f.vel.x = -0.6f + dist01_(rng_) * 1.2f;
    f.vel.y = -2.2f - dist01_(rng_) * 2.0f;
    f.vel.z = 0.4f + dist01_(rng_) * 0.6f;
    f.size = 0.02f + dist01_(rng_) * 0.04f;
}

void SnowParticles::Update(float dt) {
    for (auto& f : flakes_) {
        f.pos.x += f.vel.x * dt;
        f.pos.y += f.vel.y * dt;
        f.pos.z += f.vel.z * dt;

        if (f.pos.y < -0.2f || f.pos.z > 8.0f) {
            Respawn(f);
        }
    }
}

void SnowParticles::Draw() {
    for (const auto& f : flakes_) {
        DrawSphereEx(f.pos, f.size, 3, 3, WHITE);
    }
}

} // namespace sb
