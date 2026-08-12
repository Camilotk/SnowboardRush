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

namespace sb {

struct Ramp {
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    float coursePos = 0.0f;
    float halfWidth = 2.0f;   // x extent of the ramp surface
    float length = 8.0f;      // z extent (used for trigger zone)
    bool used = false;        // has the player already launched from it
    bool active = false;
};

} // namespace sb
