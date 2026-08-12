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

in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform float uOffset;

out vec4 finalColor;

void main()
{
    // Tile the snow noise and scroll it along the slope so the surface
    // visibly streams toward the camera as the player descends.
    vec2 uv = fragTexCoord * 50.0 + vec2(0.0, uOffset);

    // Groomed-snow bands (corduroy): a subtle repeating reference pattern
    // so the eye can measure speed against the surface, like Sonic's
    // checkered ground. Period ~10 world units, scrolls with uOffset.
    float band = step(0.5, fract(uv.y * 4.0));
    vec3 col = texture(texture0, uv).rgb;
    col *= 0.86 + 0.14 * band;

    finalColor = vec4(col, 1.0);
}
