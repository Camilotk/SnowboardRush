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
    finalColor = texture(texture0, uv);
}
