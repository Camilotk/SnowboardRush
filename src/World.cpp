#include "World.h"

#include <algorithm>

namespace sb {

namespace {
constexpr float kInnerWidth = COURSE_HALF_WIDTH - 1.0f;
constexpr float kLanes[6] = {
    -5.0f / 6.0f * kInnerWidth, -0.5f * kInnerWidth, -1.0f / 6.0f * kInnerWidth,
     1.0f / 6.0f * kInnerWidth,  0.5f * kInnerWidth,  5.0f / 6.0f * kInnerWidth
};
}

World::World() : rng_(std::random_device{}()), dist01_(0.0f, 1.0f) {
    obstacles_.resize(90);
    collectibles_.resize(140);
    ramps_.resize(14);
    mounds_.resize(90);
    markers_.resize(64);
    Reset();
}

float World::Rand01() {
    return dist01_(rng_);
}

int World::RandInt(int lo, int hi) {
    return lo + static_cast<int>(Rand01() * (hi - lo + 1));
}

void World::Reset() {
    distance_ = 0.0f;
    nextSpawnPos_ = 14.0f;
    clearUntil_ = 0.0f;
    nextMarkerPos_ = 0.0f;

    for (auto& o : obstacles_) o.active = false;
    for (auto& c : collectibles_) c.active = false;
    for (auto& r : ramps_) r.active = false;
    for (auto& m : mounds_) m.active = false;
    for (auto& k : markers_) k.active = false;
}

void World::SpawnObstacle(ObstacleType type, float x, float coursePos) {
    for (auto& o : obstacles_) {
        if (o.active) continue;
        o.type = type;
        o.radius = ObstacleRadius(type);
        o.clearHeight = ObstacleClearHeight(type);
        o.coursePos = coursePos;
        o.position = { x, SLOPE * (distance_ - coursePos), distance_ - coursePos };
        o.yaw = Rand01() * 360.0f;
        o.active = true;
        return;
    }
}

void World::SpawnCoin(float x, float y, float coursePos) {
    for (auto& c : collectibles_) {
        if (c.active) continue;
        c.coursePos = coursePos;
        c.height = y;
        const float cz = distance_ - coursePos;
        c.position = { x, SLOPE * cz + y, cz };
        c.spin = Rand01() * 360.0f;
        c.bobPhase = Rand01() * 6.28318f;
        c.active = true;
        return;
    }
}

void World::SpawnRamp(float x, float coursePos) {
    for (auto& r : ramps_) {
        if (r.active) continue;
        r.coursePos = coursePos;
        r.position = { x, SLOPE * (distance_ - coursePos), distance_ - coursePos };
        r.halfWidth = 2.0f;
        r.length = 8.0f;
        r.used = false;
        r.active = true;
        return;
    }
}

void World::SpawnMound(float x, float coursePos) {
    for (auto& m : mounds_) {
        if (m.active) continue;
        m.coursePos = coursePos;
        m.position = { x, SLOPE * (distance_ - coursePos), distance_ - coursePos };
        m.radius = 0.7f + Rand01() * 0.4f;
        m.active = true;
        return;
    }
}

void World::SpawnMarker(float x, float coursePos) {
    for (auto& k : markers_) {
        if (k.active) continue;
        k.coursePos = coursePos;
        k.position = { x, SLOPE * (distance_ - coursePos), distance_ - coursePos };
        k.active = true;
        return;
    }
}

void World::SpawnCoinLine(float coursePos) {
    const float x = kLanes[RandInt(0, 5)];
    const float y = 0.9f;
    const int count = 5;
    for (int i = 0; i < count; ++i) {
        SpawnCoin(x, y, coursePos - static_cast<float>(i) * 1.3f);
    }
}

void World::SpawnRampRow(float coursePos) {
    clearUntil_ = coursePos + RAMP_LANDING_CLEAR;
    const float x = kLanes[RandInt(2, 3)]; // near-center ramp
    SpawnRamp(x, coursePos);
    // Coins floating along the launch trajectory, slightly ahead of the ramp.
    for (int i = 0; i < 5; ++i) {
        const float cy = 2.2f + static_cast<float>(i) * 0.75f;
        const float cz = coursePos + 1.0f + static_cast<float>(i) * 1.6f;
        SpawnCoin(x, cy, cz);
    }
}

void World::SpawnSnowPatch(float coursePos) {
    const int count = 1 + RandInt(0, 2); // 1..3 mounds forming a soft slow zone
    for (int i = 0; i < count; ++i) {
        const float x = (Rand01() * 2.0f - 1.0f) * kInnerWidth;
        const float cz = coursePos + (Rand01() * 2.0f - 1.0f) * 1.5f;
        SpawnMound(x, cz);
    }
}

void World::SpawnRow(float coursePos, float difficulty) {
    // Keep the landing zone after a ramp free of obstacles so it isn't a trap.
    if (coursePos < clearUntil_) {
        SpawnCoinLine(coursePos);
        return;
    }

    const float roll = Rand01();

    if (roll < 0.12f) {
        SpawnRampRow(coursePos);
        return;
    }
    if (roll < 0.40f) {
        SpawnCoinLine(coursePos);
        return;
    }
    if (roll < 0.50f) {
        SpawnSnowPatch(coursePos);
        return;
    }
    if (roll >= 0.85f) {
        return; // open stretch - breathing room
    }

    // Obstacle row: block 1..4 of 6 lanes, always leaving at least two open.
    int blocked = 1 + static_cast<int>(difficulty * 2.0f) + (Rand01() < 0.3f ? 1 : 0);
    blocked = std::min(blocked, 4);

    int order[6] = { 0, 1, 2, 3, 4, 5 };
    std::shuffle(order, order + 6, rng_);

    for (int i = 0; i < blocked; ++i) {
        const int lane = order[i];
        const float x = kLanes[lane] + (Rand01() * 2.0f - 1.0f) * 0.4f;

        ObstacleType type;
        const float t = Rand01();
        if (t < 0.45f) type = ObstacleType::Tree;
        else if (t < 0.75f) type = ObstacleType::Rock;
        else type = ObstacleType::Barrier;

        // Ease the player in early on.
        if (difficulty < 0.2f && type == ObstacleType::Barrier) type = ObstacleType::Tree;

        SpawnObstacle(type, x, coursePos);
    }

    // Occasionally sprinkle a coin in a guaranteed-open lane.
    if (Rand01() < 0.35f) {
        const int openLane = order[blocked + RandInt(0, 5 - blocked)];
        SpawnCoin(kLanes[openLane], 0.9f, coursePos);
    }
}

void World::Update(float dt, float speed) {
    distance_ += speed * dt;

    const float difficulty = std::min(1.0f, distance_ / 4500.0f);

    // Keep the far horizon populated.
    while (nextSpawnPos_ < distance_ + SPAWN_DIST) {
        SpawnRow(nextSpawnPos_, difficulty);
        const float gap = 24.0f - difficulty * 8.0f + Rand01() * 4.0f;
        nextSpawnPos_ += gap;
        // Add a decorative mound to the side every few rows.
        if (Rand01() < 0.5f) {
            const float side = (Rand01() < 0.5f ? -1.0f : 1.0f) * (kInnerWidth + Rand01() * 3.0f);
            SpawnMound(side, nextSpawnPos_ - gap * 0.5f);
        }
    }

    // Boundary flag poles, spaced along both edges.
    while (nextMarkerPos_ < distance_ + SPAWN_DIST) {
        SpawnMarker(-COURSE_EDGE_X, nextMarkerPos_);
        SpawnMarker( COURSE_EDGE_X, nextMarkerPos_);
        nextMarkerPos_ += MARKER_SPACING;
    }

    // Move every entity and recycle those that fell behind the camera.
    for (auto& o : obstacles_) {
        if (!o.active) continue;
        o.position.z = distance_ - o.coursePos;
        o.position.y = SLOPE * o.position.z;
        if (o.position.z > DESPAWN_Z) o.active = false;
    }
    for (auto& c : collectibles_) {
        if (!c.active) continue;
        c.position.z = distance_ - c.coursePos;
        c.position.y = SLOPE * c.position.z + c.height;
        c.spin += 90.0f * dt;
        if (c.position.z > DESPAWN_Z) c.active = false;
    }
    for (auto& r : ramps_) {
        if (!r.active) continue;
        r.position.z = distance_ - r.coursePos;
        r.position.y = SLOPE * r.position.z;
        if (r.position.z > DESPAWN_Z) r.active = false;
    }
    for (auto& m : mounds_) {
        if (!m.active) continue;
        m.position.z = distance_ - m.coursePos;
        m.position.y = SLOPE * m.position.z;
        if (m.position.z > DESPAWN_Z) m.active = false;
    }
    for (auto& k : markers_) {
        if (!k.active) continue;
        k.position.z = distance_ - k.coursePos;
        k.position.y = SLOPE * k.position.z;
        if (k.position.z > DESPAWN_Z) k.active = false;
    }
}

} // namespace sb
