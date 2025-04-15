#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // Texture coordinate from vertex shader

uniform sampler2D textureAtlas;
uniform float atlasSize; // Size of the atlas

void main() {
    vec2 uv = TexCoord;
    FragColor = texture(textureAtlas, uv);
}
