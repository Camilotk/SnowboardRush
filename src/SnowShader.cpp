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

#include "SnowShader.h"

#include "raymath.h"

namespace sb {

SnowShaderSettings MakePowderPreset() {
    SnowShaderSettings s;
    // Bright, soft, high coverage, gentle detail, restrained specular.
    s.snowAccumulation = 1.30f;
    s.snowSlopeThreshold = 0.15f;
    s.snowSlopeSoftness = 0.35f;
    s.macroNoiseScale = 0.035f;
    s.mediumNoiseScale = 0.20f;
    s.detailNoiseScale = 2.2f;
    s.windDirection = { 1.0f, 0.3f };
    s.windStrength = 0.55f;
    s.windScale = 0.65f;
    s.sparkleDensity = 0.30f;
    s.sparkleIntensity = 1.0f;
    s.sparkleThreshold = 28.0f;
    s.glitterStrength = 0.30f;
    s.fresnelStrength = 0.22f;
    s.fresnelPower = 3.2f;
    s.snowDetailNormalStrength = 0.32f;
    return s;
}

SnowShaderSettings MakePackedPreset() {
    SnowShaderSettings s;
    // Darker, smoother, stronger directional reflection, less powder noise.
    s.snowAccumulation = 1.10f;
    s.snowSlopeThreshold = 0.25f;
    s.snowSlopeSoftness = 0.25f;
    s.macroNoiseScale = 0.030f;
    s.mediumNoiseScale = 0.16f;
    s.detailNoiseScale = 1.4f;
    s.windDirection = { 1.0f, 0.1f };
    s.windStrength = 0.35f;
    s.windScale = 0.45f;
    s.sparkleDensity = 0.18f;
    s.sparkleIntensity = 0.8f;
    s.sparkleThreshold = 20.0f;
    s.glitterStrength = 0.55f;
    s.fresnelStrength = 0.20f;
    s.fresnelPower = 3.0f;
    s.snowDetailNormalStrength = 0.22f;
    s.snowBrightness = 0.88f;
    return s;
}

SnowShaderSettings MakeIcyPreset() {
    SnowShaderSettings s;
    // Colder, stronger sparkle + fresnel, smoother, more directional glitter.
    s.sunSnowColor = { 0.92f, 0.96f, 1.00f };
    s.shadowSnowColor = { 0.42f, 0.56f, 0.76f };
    s.snowAccumulation = 1.15f;
    s.snowSlopeThreshold = 0.30f;
    s.snowSlopeSoftness = 0.22f;
    s.macroNoiseScale = 0.028f;
    s.mediumNoiseScale = 0.14f;
    s.detailNoiseScale = 1.1f;
    s.windDirection = { 1.0f, 0.5f };
    s.windStrength = 0.40f;
    s.windScale = 0.50f;
    s.sparkleDensity = 0.50f;
    s.sparkleIntensity = 1.6f;
    s.sparkleThreshold = 16.0f;
    s.glitterStrength = 0.65f;
    s.fresnelStrength = 0.40f;
    s.fresnelPower = 2.6f;
    s.snowDetailNormalStrength = 0.18f;
    s.snowBrightness = 0.90f;
    return s;
}

bool SnowShader::Load(const char* vsPath, const char* fsPath) {
    shader = LoadShader(vsPath, fsPath);
    if (shader.id == 0) {
        TraceLog(LOG_ERROR, "SNOW SHADER: failed to load (%s / %s)", vsPath, fsPath);
        return false;
    }

    loc_lightDirection = GetShaderLocation(shader, "lightDirection");
    loc_lightColor = GetShaderLocation(shader, "lightColor");
    loc_lightIntensity = GetShaderLocation(shader, "lightIntensity");
    loc_ambientColor = GetShaderLocation(shader, "ambientColor");
    loc_ambientIntensity = GetShaderLocation(shader, "ambientIntensity");
    loc_sunSnowColor = GetShaderLocation(shader, "sunSnowColor");
    loc_shadowSnowColor = GetShaderLocation(shader, "shadowSnowColor");
    loc_snowBrightness = GetShaderLocation(shader, "snowBrightness");
    loc_baseAlbedo = GetShaderLocation(shader, "baseAlbedo");
    loc_snowAccumulation = GetShaderLocation(shader, "snowAccumulation");
    loc_snowSlopeThreshold = GetShaderLocation(shader, "snowSlopeThreshold");
    loc_snowSlopeSoftness = GetShaderLocation(shader, "snowSlopeSoftness");
    loc_macroNoiseScale = GetShaderLocation(shader, "macroNoiseScale");
    loc_mediumNoiseScale = GetShaderLocation(shader, "mediumNoiseScale");
    loc_detailNoiseScale = GetShaderLocation(shader, "detailNoiseScale");
    loc_windDirection = GetShaderLocation(shader, "windDirection");
    loc_windStrength = GetShaderLocation(shader, "windStrength");
    loc_windScale = GetShaderLocation(shader, "windScale");
    loc_sparkleDensity = GetShaderLocation(shader, "sparkleDensity");
    loc_sparkleIntensity = GetShaderLocation(shader, "sparkleIntensity");
    loc_sparkleThreshold = GetShaderLocation(shader, "sparkleThreshold");
    loc_sparkleScale = GetShaderLocation(shader, "sparkleScale");
    loc_glitterStrength = GetShaderLocation(shader, "glitterStrength");
    loc_fresnelStrength = GetShaderLocation(shader, "fresnelStrength");
    loc_fresnelPower = GetShaderLocation(shader, "fresnelPower");
    loc_snowDetailNormalStrength = GetShaderLocation(shader, "snowDetailNormalStrength");
    loc_detailFadeStart = GetShaderLocation(shader, "detailFadeStart");
    loc_detailFadeEnd = GetShaderLocation(shader, "detailFadeEnd");
    loc_trackInfluence = GetShaderLocation(shader, "trackInfluence");
    loc_useTrackMask = GetShaderLocation(shader, "useTrackMask");
    loc_cameraPosition = GetShaderLocation(shader, "cameraPosition");
    loc_time = GetShaderLocation(shader, "time");
    loc_snowWorldOffset = GetShaderLocation(shader, "snowWorldOffset");
    loc_debugMode = GetShaderLocation(shader, "debugMode");

    TraceLog(LOG_INFO, "SNOW SHADER: loaded [ID %i]", shader.id);

    // Static "reserved" uniforms.
    const Vector3 kZero{ 0.0f, 0.0f, 0.0f };
    const float kZeroF = 0.0f;
    const int kUseTrackMask = 0;
    SetShaderValue(shader, loc_snowWorldOffset, &kZero, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_useTrackMask, &kUseTrackMask, SHADER_UNIFORM_INT);
    SetShaderValue(shader, loc_trackInfluence, &kZeroF, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_debugMode, &debugMode_, SHADER_UNIFORM_INT);
    SetShaderValue(shader, loc_time, &kZeroF, SHADER_UNIFORM_FLOAT);

    return true;
}

void SnowShader::Unload() {
    if (shader.id != 0) {
        UnloadShader(shader);
        shader.id = 0;
    }
}

void SnowShader::ApplySettings(const SnowShaderSettings& s) {
    if (!IsReady()) return;

    // Directional light: a static sun direction (light travels this way).
    // Chosen to match the sun sphere drawn in the sky (up, ahead, to the right).
    const Vector3 sunPosition{ 60.0f, 70.0f, -220.0f };
    const Vector3 sunTarget{ 0.0f, 0.0f, 0.0f };
    const Vector3 lightDir = Vector3Normalize(Vector3Subtract(sunTarget, sunPosition));

    SetShaderValue(shader, loc_lightDirection, &lightDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_lightColor, &s.lightColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_lightIntensity, &s.lightIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_ambientColor, &s.ambientColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_ambientIntensity, &s.ambientIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_sunSnowColor, &s.sunSnowColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_shadowSnowColor, &s.shadowSnowColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_snowBrightness, &s.snowBrightness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_baseAlbedo, &s.baseAlbedo, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_snowAccumulation, &s.snowAccumulation, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_snowSlopeThreshold, &s.snowSlopeThreshold, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_snowSlopeSoftness, &s.snowSlopeSoftness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_macroNoiseScale, &s.macroNoiseScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_mediumNoiseScale, &s.mediumNoiseScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_detailNoiseScale, &s.detailNoiseScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_windDirection, &s.windDirection, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, loc_windStrength, &s.windStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_windScale, &s.windScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_sparkleDensity, &s.sparkleDensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_sparkleIntensity, &s.sparkleIntensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_sparkleThreshold, &s.sparkleThreshold, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_sparkleScale, &s.sparkleScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_glitterStrength, &s.glitterStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_fresnelStrength, &s.fresnelStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_fresnelPower, &s.fresnelPower, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_snowDetailNormalStrength, &s.snowDetailNormalStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_detailFadeStart, &s.detailFadeStart, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, loc_detailFadeEnd, &s.detailFadeEnd, SHADER_UNIFORM_FLOAT);
}

void SnowShader::UpdatePerFrame(const Vector3& cameraPosition, float time) {
    if (!IsReady()) return;
    SetShaderValue(shader, loc_cameraPosition, &cameraPosition, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, loc_time, &time, SHADER_UNIFORM_FLOAT);
}

void SnowShader::SetDebugMode(int mode) {
    debugMode_ = mode;
    if (debugMode_ < 0) debugMode_ = 11;
    if (debugMode_ > 11) debugMode_ = 0;
    if (IsReady()) SetShaderValue(shader, loc_debugMode, &debugMode_, SHADER_UNIFORM_INT);
}

void SnowShader::CycleDebugMode(int direction) {
    int next = debugMode_ + (direction > 0 ? 1 : -1);
    if (next < 0) next = 11;
    if (next > 11) next = 0;
    SetDebugMode(next);
}

} // namespace sb
