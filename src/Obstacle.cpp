#include "Obstacle.h"

namespace sb {

float ObstacleRadius(ObstacleType type) {
    switch (type) {
        case ObstacleType::Tree: return 0.55f;
        case ObstacleType::Rock: return 0.85f;
        case ObstacleType::Barrier: return 0.70f;
    }
    return 0.60f;
}

float ObstacleClearHeight(ObstacleType type) {
    switch (type) {
        case ObstacleType::Tree: return 3.5f;    // cannot be jumped
        case ObstacleType::Rock: return 0.55f;   // jumpable
        case ObstacleType::Barrier: return 3.5f; // cannot be jumped
    }
    return 1.0f;
}

} // namespace sb
