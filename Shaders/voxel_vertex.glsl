#version 330 core
layout(location = 0) in vec3 aPos;        // Vertex position
layout(location = 3) in vec2 aTexCoord;   // Texture coordinates
layout(location = 1) in vec3 instancePosition; // Instance position
layout(location = 2) in uint blockId;     // Block ID
layout(location = 4) in vec3 aNormal;     // Vertex normal

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoord;
out uint BlockId;
out vec3 FragPos;     // Fragment position in world space
out vec3 Normal;      // Normal in world space

void main() {
    vec3 worldPos = aPos + instancePosition;
    FragPos = worldPos;
    
    // Transform normal to world space
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    TexCoord = aTexCoord;
    BlockId = blockId;
}
