#version 330 core
layout(location = 0) in vec3 aPos; // Vertex position
layout(location = 3) in vec2 aTexCoord; // Texture coordinates
layout(location = 1) in vec3 instancePosition; // Instance position
layout(location = 2) in uint blockId; // Block ID

uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
out uint BlockId;

void main() {
    // Calculate the final position of the vertex
    gl_Position = projection * view * vec4(aPos + instancePosition, 1.0);
    TexCoord = aTexCoord;
    BlockId = blockId;
}
