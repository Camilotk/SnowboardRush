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

#include "SoundManager.h"

#include "Constants.h"

#include <cmath>
#include <random>

namespace sb {

namespace {
constexpr int kSampleRate = 22050;
constexpr float kPi = 3.14159265358979323846f;

float envelope(float t, float total) {
    if (t >= total) return 0.0f;
    return 1.0f - (t / total);
}
} // namespace

Sound SoundManager::BuildSound(const std::function<float(float, float)>& gen, float seconds) {
    const int frameCount = static_cast<int>(seconds * kSampleRate);

    Wave wave = { 0 };
    wave.frameCount = static_cast<unsigned int>(frameCount);
    wave.sampleRate = kSampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc(sizeof(short) * static_cast<unsigned int>(frameCount));

    auto* samples = static_cast<short*>(wave.data);
    for (int i = 0; i < frameCount; ++i) {
        const float t = static_cast<float>(i) / kSampleRate;
        float s = gen(t, seconds);
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
        samples[i] = static_cast<short>(s * 30000.0f);
    }

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

void SoundManager::Init(const char* windPath) {
    InitAudioDevice();

    // Coin pickup: quick rising chime.
    coin_ = BuildSound([](float t, float total) {
        const float f = 700.0f + 800.0f * (t / total);
        return std::sin(2.0f * kPi * f * t) * envelope(t, total);
    }, 0.16f);

    // Jump: short rising whoosh.
    jump_ = BuildSound([](float t, float total) {
        const float f = 250.0f + 500.0f * (t / total);
        return std::sin(2.0f * kPi * f * t) * envelope(t, total) * 0.8f;
    }, 0.2f);

    // Ramp launch: longer rising sweep.
    ramp_ = BuildSound([](float t, float total) {
        const float f = 180.0f + 700.0f * (t / total);
        return std::sin(2.0f * kPi * f * t) * envelope(t, total);
    }, 0.4f);

    // Crash: descending noise burst.
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    crash_ = BuildSound([](float t, float total) {
        const float f = 400.0f - 200.0f * (t / total);
        const float wobble = std::sin(2.0f * kPi * f * t);
        return (wobble * 0.5f + dist(rng) * 0.5f) * envelope(t, total);
    }, 0.45f);

    // Wind: a looping ambient bed loaded from a bundled recording.
    windMusic_ = LoadMusicStream(windPath);
    if (IsMusicValid(windMusic_)) {
        windMusic_.looping = true;
        SetMusicVolume(windMusic_, 0.0f);
        PlayMusicStream(windMusic_);
        windReady_ = true;
    }

    ready_ = true;
}

void SoundManager::Shutdown() {
    if (!ready_) return;
    UnloadSound(coin_);
    UnloadSound(jump_);
    UnloadSound(crash_);
    UnloadSound(ramp_);
    if (windReady_) {
        UnloadMusicStream(windMusic_);
        windReady_ = false;
    }
    CloseAudioDevice();
    ready_ = false;
}

void SoundManager::UpdateWind(float speedT) {
    if (!windReady_) return;
    UpdateMusicStream(windMusic_);
    if (speedT < 0.0f) speedT = 0.0f;
    if (speedT > 1.0f) speedT = 1.0f;
    SetMusicVolume(windMusic_, 0.4f * speedT * SPEED_FX_WIND);
}

void SoundManager::PlayCoin() { if (ready_) PlaySound(coin_); }
void SoundManager::PlayJump() { if (ready_) PlaySound(jump_); }
void SoundManager::PlayCrash() { if (ready_) PlaySound(crash_); }
void SoundManager::PlayRamp() { if (ready_) PlaySound(ramp_); }

} // namespace sb
