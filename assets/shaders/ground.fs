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
