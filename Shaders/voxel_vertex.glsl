#version 330 core
layout(location = 0) in vec3 aPos;        // Vertex position
layout(location = 3) in vec2 aTexCoord;   // Base texture coordinates
layout(location = 1) in vec3 instancePosition; // Instance position
layout(location = 2) in uint blockId;     // Block ID
layout(location = 4) in vec3 aNormal;     // Vertex normal

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;
uniform float atlasSize;

out vec2 TexCoord;
flat out uint BlockId;  // Added flat qualifier
out vec3 FragPos;     
out vec3 Normal;      
out vec4 FragPosLightSpace;

vec2 getBlockTexCoords(uint blockId, vec3 normal) {
    vec2 baseCoord;
    
    if (normal.y > 0.5) {         // Top face
        if (blockId == 1u) {
            baseCoord = vec2(0, 0);  // Grass top
        } else if (blockId == 2u) {
            baseCoord = vec2(2, 0);  // Dirt
        } else if (blockId == 3u) {
            baseCoord = vec2(1, 0);  // Stone
        } else if (blockId == 4u) {
            baseCoord = vec2(4, 1);  // Wood top
        } else if (blockId == 5u) {
            baseCoord = vec2(4, 3);  // Leaves
        } else if (blockId == 6u) {
            baseCoord = vec2(7, 0);  // Lava
        } else if (blockId == 7u) {
            baseCoord = vec2(0, 9);  // Water
        } else if (blockId == 8u) {
            baseCoord = vec2(2, 1);  // Sand
        }
    }
    else if (normal.y < -0.5) {   // Bottom face
        if (blockId == 1u) {
            baseCoord = vec2(2, 0);  // Dirt for grass bottom
        } else if (blockId == 2u) {
            baseCoord = vec2(2, 0);  // Dirt
        } else if (blockId == 3u) {
            baseCoord = vec2(1, 0);  // Stone
        } else if (blockId == 4u) {
            baseCoord = vec2(4, 1);  // Wood top
        } else if (blockId == 5u) {
            baseCoord = vec2(4, 3);  // Leaves
        } else if (blockId == 6u) {
            baseCoord = vec2(7, 0);  // Lava
        } else if (blockId == 7u) {
            baseCoord = vec2(0, 9);  // Water
        } else if (blockId == 8u) {
            baseCoord = vec2(2, 1);  // Sand
        }
    }
    else {                        // Side faces
        if (blockId == 1u) {
            baseCoord = vec2(3, 0);  // Grass side
        } else if (blockId == 2u) {
            baseCoord = vec2(2, 0);  // Dirt
        } else if (blockId == 3u) {
            baseCoord = vec2(1, 0);  // Stone
        } else if (blockId == 4u) {
            baseCoord = vec2(4, 1);  // Wood side
        } else if (blockId == 5u) {
            baseCoord = vec2(5, 3);  // Leaves
        } else if (blockId == 6u) {
            baseCoord = vec2(7, 0);  // Lava
        } else if (blockId == 7u) {
            baseCoord = vec2(0, 9);  // Water
        } else if (blockId == 8u) {
            baseCoord = vec2(2, 1);  // Sand
        }
    }
    
    // Scale coordinates to atlas size
    return (baseCoord + aTexCoord) / atlasSize;
}

void main() {
    // Apply model transformation to get world position
    vec3 worldPos = aPos + instancePosition;
    FragPos = vec3(model * vec4(worldPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Calculate light space fragment position for shadows
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    
    // Calculate final position in clip space
    gl_Position = projection * view * vec4(FragPos, 1.0);
    
    TexCoord = getBlockTexCoords(blockId, aNormal);
    BlockId = blockId;
}
