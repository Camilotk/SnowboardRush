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

// Tunable parameters for the procedural snow shader.
struct SnowShaderSettings {
    Vector3 sunSnowColor    = { 0.95f, 0.97f, 1.00f };
    Vector3 shadowSnowColor = { 0.48f, 0.62f, 0.78f };

    Vector3 lightColor      = { 1.00f, 0.97f, 0.92f };
    Vector3 ambientColor    = { 0.85f, 0.92f, 1.00f };

    float lightIntensity    = 1.00f;
    float ambientIntensity  = 0.85f;

    float snowBrightness    = 1.00f;

    float snowAccumulation  = 1.20f;
    float snowSlopeThreshold = 0.30f;
    float snowSlopeSoftness = 0.30f;

    float macroNoiseScale   = 0.035f;
    float mediumNoiseScale  = 0.25f;
    float detailNoiseScale  = 2.0f;

    Vector2 windDirection   = { 1.0f, 0.3f };
    float windStrength      = 0.5f;
    float windScale         = 0.6f;

    float sparkleDensity    = 0.35f;
    float sparkleIntensity  = 1.2f;
    float sparkleThreshold  = 24.0f;
    float sparkleScale      = 1.5f;

    float glitterStrength   = 0.35f;

    float fresnelStrength   = 0.22f;
    float fresnelPower      = 3.0f;

    float snowDetailNormalStrength = 0.35f;

    float detailFadeStart   = 25.0f;
    float detailFadeEnd     = 120.0f;

    Vector3 baseAlbedo      = { 0.72f, 0.77f, 0.83f }; // packed snow under-layer
    float trackInfluence    = 0.0f;                     // reserved
};

// Quality presets.
SnowShaderSettings MakePowderPreset();
SnowShaderSettings MakePackedPreset();
SnowShaderSettings MakeIcyPreset();

// Owns the shader program and its cached uniform locations.
class SnowShader {
public:
    bool Load(const char* vsPath, const char* fsPath);
    void Unload();
    bool IsReady() const { return shader.id != 0; }

    // Push the entire settings block (values that rarely change).
    void ApplySettings(const SnowShaderSettings& settings);

    // Per-frame updates: actual camera position and time.
    void UpdatePerFrame(const Vector3& cameraPosition, float time);

    // Debug mode cycling (F5/F6). 0 = final render.
    void SetDebugMode(int mode);
    void CycleDebugMode(int direction);
    int DebugMode() const { return debugMode_; }

    Shader shader = {};

private:
    int loc_lightDirection = -1;
    int loc_lightColor = -1;
    int loc_lightIntensity = -1;
    int loc_ambientColor = -1;
    int loc_ambientIntensity = -1;
    int loc_sunSnowColor = -1;
    int loc_shadowSnowColor = -1;
    int loc_snowBrightness = -1;
    int loc_baseAlbedo = -1;
    int loc_snowAccumulation = -1;
    int loc_snowSlopeThreshold = -1;
    int loc_snowSlopeSoftness = -1;
    int loc_macroNoiseScale = -1;
    int loc_mediumNoiseScale = -1;
    int loc_detailNoiseScale = -1;
    int loc_windDirection = -1;
    int loc_windStrength = -1;
    int loc_windScale = -1;
    int loc_sparkleDensity = -1;
    int loc_sparkleIntensity = -1;
    int loc_sparkleThreshold = -1;
    int loc_sparkleScale = -1;
    int loc_glitterStrength = -1;
    int loc_fresnelStrength = -1;
    int loc_fresnelPower = -1;
    int loc_snowDetailNormalStrength = -1;
    int loc_detailFadeStart = -1;
    int loc_detailFadeEnd = -1;
    int loc_trackInfluence = -1;
    int loc_useTrackMask = -1;
    int loc_cameraPosition = -1;
    int loc_time = -1;
    int loc_snowWorldOffset = -1;
    int loc_debugMode = -1;

    int debugMode_ = 0;
};

} // namespace sb
