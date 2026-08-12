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
#include "Obstacle.h"
#include "Collectible.h"
#include "Ramp.h"

#include <random>
#include <vector>

namespace sb {

struct SnowMound {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float coursePos = 0.0f;
    float radius = 0.5f;
    bool active = false;
};

struct EdgeMarker {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float coursePos = 0.0f;
    bool active = false;
};

class World {
public:
    World();

    void Reset();
    void Update(float dt, float speed);

    const std::vector<Obstacle>& Obstacles() const { return obstacles_; }
    const std::vector<Collectible>& Collectibles() const { return collectibles_; }
    const std::vector<Ramp>& Ramps() const { return ramps_; }
    const std::vector<SnowMound>& Mounds() const { return mounds_; }
    const std::vector<EdgeMarker>& Markers() const { return markers_; }

    std::vector<Collectible>& Collectibles() { return collectibles_; }
    std::vector<Ramp>& Ramps() { return ramps_; }

    float Distance() const { return distance_; }

private:
    void SpawnObstacle(ObstacleType type, float x, float coursePos);
    void SpawnCoin(float x, float y, float coursePos);
    void SpawnRamp(float x, float coursePos);
    void SpawnMound(float x, float coursePos);
    void SpawnMarker(float x, float coursePos);

    void SpawnRow(float coursePos, float difficulty);
    void SpawnCoinLine(float coursePos);
    void SpawnRampRow(float coursePos);
    void SpawnSnowPatch(float coursePos);

    float Rand01();
    int RandInt(int lo, int hi);

    std::vector<Obstacle> obstacles_;
    std::vector<Collectible> collectibles_;
    std::vector<Ramp> ramps_;
    std::vector<SnowMound> mounds_;
    std::vector<EdgeMarker> markers_;

    float distance_ = 0.0f;
    float nextSpawnPos_ = 0.0f;
    float clearUntil_ = 0.0f;   // keep obstacles out of the ramp landing zone
    float nextMarkerPos_ = 0.0f;

    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist01_;
};

} // namespace sb
