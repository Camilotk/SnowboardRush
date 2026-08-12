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

// raylib default vertex attributes (see rlgl RL_DEFAULT_SHADER_ATTRIB_NAME_*).
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// raylib auto-populated matrices.
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// World-space outputs consumed by the fragment shader.
out vec3 fragWorldPosition;
out vec3 fragWorldNormal;
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // World-space position: model is the world transform for every model in
    // this game (the ground plane's slope tilt lives in matModel).
    fragWorldPosition = vec3(matModel * vec4(vertexPosition, 1.0));

    // World-space normal: matNormal = transpose(inverse(matModel)).
    fragWorldNormal = normalize(mat3(matNormal) * vertexNormal);

    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
