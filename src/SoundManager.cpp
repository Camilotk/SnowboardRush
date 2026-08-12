#include "SoundManager.h"

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

void SoundManager::Init() {
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

    ready_ = true;
}

void SoundManager::Shutdown() {
    if (!ready_) return;
    UnloadSound(coin_);
    UnloadSound(jump_);
    UnloadSound(crash_);
    UnloadSound(ramp_);
    CloseAudioDevice();
    ready_ = false;
}

void SoundManager::PlayCoin() { if (ready_) PlaySound(coin_); }
void SoundManager::PlayJump() { if (ready_) PlaySound(jump_); }
void SoundManager::PlayCrash() { if (ready_) PlaySound(crash_); }
void SoundManager::PlayRamp() { if (ready_) PlaySound(ramp_); }

} // namespace sb
