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
