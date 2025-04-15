#version 330 core
layout(location = 0) in vec3 aPos;        // Vertex position
layout(location = 3) in vec2 aTexCoord;   // Base texture coordinates
layout(location = 1) in vec3 instancePosition; // Instance position
layout(location = 2) in uint blockId;     // Block ID
layout(location = 4) in vec3 aNormal;     // Vertex normal

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform float atlasSize;

out vec2 TexCoord;
out uint BlockId;
out vec3 FragPos;     // Fragment position in world space
out vec3 Normal;      // Normal in world space

vec2 getBlockTexCoords(uint blockId, vec3 normal) {
    vec2 baseCoord;
    
    // Determine which face we're on based on the normal
    if (normal.y > 0.5) {         // Top face
        baseCoord = (blockId == 1u) ? vec2(0, 0) : vec2(2, 0);  // Grass top for block 1, dirt for others
    }
    else if (normal.y < -0.5) {   // Bottom face
        baseCoord = vec2(2, 0);   // Always dirt
    }
    else {                        // Side faces
        baseCoord = (blockId == 1u) ? vec2(3, 0) : vec2(2, 0);  // Grass side for block 1, dirt for others
    }
    
    // Apply the base UV offset within the atlas
    return (baseCoord + aTexCoord) / atlasSize;
}

void main() {
    vec3 worldPos = aPos + instancePosition;
    FragPos = worldPos;
    
    // Transform normal to world space
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    TexCoord = getBlockTexCoords(blockId, aNormal);
    BlockId = blockId;
}
