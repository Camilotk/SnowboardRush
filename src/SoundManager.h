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

#include <functional>

namespace sb {

class SoundManager {
public:
    void Init();
    void Shutdown();

    void PlayCoin();
    void PlayJump();
    void PlayCrash();
    void PlayRamp();

private:
    Sound BuildSound(const std::function<float(float, float)>& gen, float seconds);

    Sound coin_ = {};
    Sound jump_ = {};
    Sound crash_ = {};
    Sound ramp_ = {};
    bool ready_ = false;
};

} // namespace sb
