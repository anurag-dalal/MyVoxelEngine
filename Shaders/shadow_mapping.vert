#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 instancePosition;

uniform mat4 lightSpaceMatrix;

void main() {
    vec3 worldPos = aPos + instancePosition;
    gl_Position = lightSpaceMatrix * vec4(worldPos, 1.0);
}