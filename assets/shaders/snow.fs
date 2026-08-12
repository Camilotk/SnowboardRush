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

#version 330

// ---------------------------------------------------------------------------
// Advanced procedural snow shader.
//
// All snow detail is derived from WORLD-space coordinates so the pattern is
// spatially fixed and never swims as the camera or terrain move. The ground
// plane is static in this game (entities scroll past it), so a world-space
// pattern is inherently temporally stable.
// ---------------------------------------------------------------------------

in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

// ---------------------------------------------------------------------------
// Lighting
// ---------------------------------------------------------------------------
uniform vec3  lightDirection;      // direction light TRAVELS (sun -> scene); use -lightDirection for sun-facing
uniform vec3  lightColor;
uniform float lightIntensity;

uniform vec3  ambientColor;
uniform float ambientIntensity;

uniform vec3  sunSnowColor;        // illuminated snow
uniform vec3  shadowSnowColor;     // cold, blue-tinted shadowed snow

uniform float snowBrightness;

// ---------------------------------------------------------------------------
// Snow accumulation (0 = exposed under-surface, 1 = full snow)
// ---------------------------------------------------------------------------
uniform vec3  baseAlbedo;          // surface exposed where snow thins (packed snow / rock)
uniform float snowAccumulation;    // global accumulation control
uniform float snowSlopeThreshold;
uniform float snowSlopeSoftness;

// ---------------------------------------------------------------------------
// Procedural noise scales
// ---------------------------------------------------------------------------
uniform float macroNoiseScale;
uniform float mediumNoiseScale;
uniform float detailNoiseScale;

// ---------------------------------------------------------------------------
// Wind
// ---------------------------------------------------------------------------
uniform vec2  windDirection;       // unit-ish vector in world XZ
uniform float windStrength;
uniform float windScale;

// ---------------------------------------------------------------------------
// Sparkles / glitter
// ---------------------------------------------------------------------------
uniform float sparkleDensity;
uniform float sparkleIntensity;
uniform float sparkleThreshold;
uniform float sparkleScale;
uniform float glitterStrength;

// ---------------------------------------------------------------------------
// Fresnel
// ---------------------------------------------------------------------------
uniform float fresnelStrength;
uniform float fresnelPower;

// ---------------------------------------------------------------------------
// Detail normal + distance fade
// ---------------------------------------------------------------------------
uniform float snowDetailNormalStrength;
uniform float detailFadeStart;
uniform float detailFadeEnd;

// ---------------------------------------------------------------------------
// Snowboard track support (reserved; not yet bound by the game)
// ---------------------------------------------------------------------------
uniform sampler2D trackMask;
uniform float     trackInfluence;
uniform int       useTrackMask;

// ---------------------------------------------------------------------------
// Misc
// ---------------------------------------------------------------------------
uniform vec3  cameraPosition;
uniform float time;
uniform vec3  snowWorldOffset;     // reserved for recycled-chunk compensation
uniform int   debugMode;

uniform vec4  colDiffuse;

// ---------------------------------------------------------------------------
// Hash + value noise
// ---------------------------------------------------------------------------
float hash12(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float hash13(vec3 p)
{
    vec3 p3 = fract(p * 0.1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

float valueNoise2(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    float n00 = hash12(i);
    float n10 = hash12(i + vec2(1.0, 0.0));
    float n01 = hash12(i + vec2(0.0, 1.0));
    float n11 = hash12(i + vec2(1.0, 1.0));

    return mix(mix(n00, n10, u.x), mix(n01, n11, u.x), u.y);
}

float valueNoise3(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);

    float n000 = hash13(i);
    float n100 = hash13(i + vec3(1.0, 0.0, 0.0));
    float n010 = hash13(i + vec3(0.0, 1.0, 0.0));
    float n110 = hash13(i + vec3(1.0, 1.0, 0.0));
    float n001 = hash13(i + vec3(0.0, 0.0, 1.0));
    float n101 = hash13(i + vec3(1.0, 0.0, 1.0));
    float n011 = hash13(i + vec3(0.0, 1.0, 1.0));
    float n111 = hash13(i + vec3(1.0, 1.0, 1.0));

    float nx00 = mix(n000, n100, u.x);
    float nx10 = mix(n010, n110, u.x);
    float nx01 = mix(n001, n101, u.x);
    float nx11 = mix(n011, n111, u.x);

    float nxy0 = mix(nx00, nx10, u.y);
    float nxy1 = mix(nx01, nx11, u.y);

    return mix(nxy0, nxy1, u.z);
}

// ---------------------------------------------------------------------------
// Wind-swept pattern: anisotropic coordinates aligned with windDirection.
// ---------------------------------------------------------------------------
float windPattern(vec2 xz)
{
    vec2 axis = normalize(windDirection + vec2(1e-4));
    vec2 perp = vec2(-axis.y, axis.x);

    vec2 p = vec2(dot(xz, axis), dot(xz, perp));

    // Stretch along the wind axis and compress across it -> elongated ridges.
    p.x /= max(windScale, 0.001);
    p.y *= max(windScale, 0.001);

    float field = valueNoise2(p);
    float ridge = 1.0 - abs(2.0 * valueNoise2(p * 3.0 + 11.7) - 1.0);

    return field * 0.65 + ridge * 0.35;
}

// ---------------------------------------------------------------------------
// Snow accumulation mask. Reusable for terrain, rocks, ramps, props...
// ---------------------------------------------------------------------------
float CalculateSnowAmount(vec3 worldPosition, vec3 worldNormal, float macroNoise)
{
    float up = dot(normalize(worldNormal), vec3(0.0, 1.0, 0.0));

    // Top-facing -> snow, steep/vertical -> exposed under-surface.
    float slopeMask = smoothstep(snowSlopeThreshold, snowSlopeThreshold + snowSlopeSoftness, up);

    // Broad powder variation perturbs where snow collects.
    float acc = slopeMask * mix(0.8, 1.0, macroNoise);

    return clamp(acc * snowAccumulation, 0.0, 1.0);
}

void main()
{
    // --- world-space setup -------------------------------------------------
    vec3 worldPos = fragWorldPosition + snowWorldOffset;
    vec3 N = normalize(fragWorldNormal);
    vec3 V = normalize(cameraPosition - fragWorldPosition);
    vec3 L = normalize(-lightDirection);

    float dist = distance(cameraPosition, fragWorldPosition);

    // Distance-based detail reduction: keep macro, fade fine detail.
    float detailFade = 1.0 - smoothstep(detailFadeStart, detailFadeEnd, dist);

    // --- procedural noise layers (fixed in world space) --------------------
    vec3 wp = worldPos;
    float macro  = valueNoise3(wp * macroNoiseScale);
    float medium = valueNoise3(wp * mediumNoiseScale + 7.31);
    float micro  = valueNoise3(wp * detailNoiseScale + 3.17);
    float wind   = windPattern(wp.xz);

    // --- slope response (snow appearance, not accumulation) ---------------
    float upFacing = clamp(dot(N, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);
    float slope = 1.0 - smoothstep(0.35, 0.85, upFacing); // 0 flat -> 1 steep

    // --- procedural detail normal ------------------------------------------
    // Cheap tangent-space height-field gradient from the micro noise.
    vec3 up = (abs(N.y) < 0.999) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 T = normalize(cross(up, N));
    vec3 B = cross(N, T);
    float eps = 0.06;
    float hT = valueNoise3((wp + T * eps) * detailNoiseScale + 3.17);
    float hB = valueNoise3((wp + B * eps) * detailNoiseScale + 3.17);
    vec3 Ndetail = normalize(N - (T * (hT - micro) + B * (hB - micro)) * snowDetailNormalStrength * detailFade);
    N = Ndetail;

    // --- lighting -----------------------------------------------------------
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);

    // Soft half-lambert wrap: sun amount in 0..1, never a hard shadow edge.
    float sun = clamp((NdotL + 0.25) / 1.25, 0.0, 1.0);

    // Cold sky fill (constant) and warm sun tint, mixed by the sun amount.
    vec3 illum = mix(ambientColor * ambientIntensity, lightColor * lightIntensity, sun);

    // Artist-controlled snow tone: cold blue in shade, bright near-white in sun.
    vec3 litSnow = mix(shadowSnowColor, sunSnowColor, sun) * illum * snowBrightness;

    // --- accumulation + under-surface blend --------------------------------
    float snowAmount = CalculateSnowAmount(worldPos, N, macro);

    // Powder variation modulates the snow color slightly (packed vs loose).
    float powder = mix(-0.5, 0.5, macro) + mix(-0.35, 0.35, medium);
    litSnow *= 1.0 + powder * 0.10;

    // Steeper snow is a touch darker and more compacted.
    litSnow *= 1.0 - slope * 0.12;

    // Wind-swept regions are slightly brighter and smoother (compacted).
    litSnow += (wind - 0.5) * windStrength * 0.16;

    // Under-surface (exposed) lit with the same lights for consistency.
    vec3 litBase = baseAlbedo * illum * snowBrightness;

    vec3 albedo = mix(litBase, litSnow, snowAmount);

    // --- sparkles -----------------------------------------------------------
    // World-space stable crystalline glints, sparse + distance-filtered.
    float sparkle = 0.0;
    {
        vec3 cell = floor(worldPos * sparkleScale);
        vec3 rnd  = vec3(hash13(cell), hash13(cell + 5.7), hash13(cell + 13.1));
        vec3 d    = fract(worldPos * sparkleScale) - rnd;
        float d2  = dot(d, d);

        float exists = step(1.0 - sparkleDensity, hash13(cell + 29.3));
        float glint  = 1.0 - smoothstep(0.0, 0.05, d2);

        // Sun/surface/view alignment gate + distance fade to avoid flicker.
        float sunGate   = smoothstep(0.25, 0.9, NdotL);
        float alignGate = pow(max(dot(reflect(-L, N), V), 0.0), sparkleThreshold);

        sparkle = glint * exists * sunGate * alignGate * detailFade;
    }

    // --- directional sun glitter (broad, subtle) ----------------------------
    vec3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0);
    float glitter = pow(NdotH, 64.0) * glitterStrength * NdotL * (0.5 + 0.5 * detailFade);

    // --- fresnel ------------------------------------------------------------
    float fresnel = pow(1.0 - NdotV, fresnelPower) * fresnelStrength;

    // --- snowboard track support (reserved; inactive until a mask is set) ---
    float track = 0.0;
    if (useTrackMask == 1) {
        track = texture(trackMask, fragTexCoord).r * trackInfluence;
    }

    // --- final composition ---------------------------------------------------
    vec3 color = albedo;
    color += vec3(1.0, 0.99, 0.95) * sparkle * sparkleIntensity;
    color += vec3(1.0) * glitter;
    color += vec3(0.82, 0.90, 1.00) * fresnel * 0.6 * (0.3 + 0.7 * NdotL);

    // Tracked snow: darker, bluer, less sparkle (compressed snow).
    color = mix(color, color * vec3(0.80, 0.84, 0.92), track);

    color *= colDiffuse.rgb;
    color = min(color, vec3(1.0));

    // -------------------------------------------------------------------------
    // Debug visualization
    // -------------------------------------------------------------------------
    if (debugMode == 1) { color = N * 0.5 + 0.5; }
    else if (debugMode == 2) { color = vec3(smoothstep(0.0, 1.0, upFacing)); }
    else if (debugMode == 3) { color = vec3(snowAmount); }
    else if (debugMode == 4) { color = vec3(macro); }
    else if (debugMode == 5) { color = vec3(medium); }
    else if (debugMode == 6) { color = vec3(micro); }
    else if (debugMode == 7) { color = vec3(sparkle); }
    else if (debugMode == 8) { color = vec3(NdotL); }
    else if (debugMode == 9) { color = vec3(fresnel); }
    else if (debugMode == 10) { color = vec3(detailFade); }
    else if (debugMode == 11) { color = vec3(track); }

    finalColor = vec4(color, 1.0);
}
