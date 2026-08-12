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
