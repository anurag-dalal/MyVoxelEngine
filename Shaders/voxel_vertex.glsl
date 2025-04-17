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
        } else if (blockId == 9u) {
            baseCoord = vec2(2, 3);  // Snow top
        } else if (blockId == 10u) {
            baseCoord = vec2(1, 1);  // Gravel top
        } else if (blockId == 11u) {
            baseCoord = vec2(3, 4);  // Ice top
        } else if (blockId == 12u) {
            baseCoord = vec2(2, 2);  // Alpine Grass top
        } else if (blockId == 13u) {
            baseCoord = vec2(1, 9);  // Alpine Lake top
        } else if (blockId == 14u) {
            baseCoord = vec2(3, 1);  // Clay top
        } else if (blockId == 15u) {
            baseCoord = vec2(0, 1);  // Cobblestone top
        } else if (blockId == 16u) {
            baseCoord = vec2(2, 3);  // Snowy Dirt top
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
        } else if (blockId == 9u) {
            baseCoord = vec2(2, 0);  // Snow bottom (dirt)
        } else if (blockId == 10u) {
            baseCoord = vec2(1, 1);  // Gravel bottom
        } else if (blockId == 11u) {
            baseCoord = vec2(3, 4);  // Ice bottom
        } else if (blockId == 12u) {
            baseCoord = vec2(2, 0);  // Alpine Grass bottom (dirt)
        } else if (blockId == 13u) {
            baseCoord = vec2(1, 9);  // Alpine Lake bottom
        } else if (blockId == 14u) {
            baseCoord = vec2(3, 1);  // Clay bottom
        } else if (blockId == 15u) {
            baseCoord = vec2(0, 1);  // Cobblestone bottom
        } else if (blockId == 16u) {
            baseCoord = vec2(2, 0);  // Snowy Dirt bottom (dirt)
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
        } else if (blockId == 9u) {
            baseCoord = vec2(2, 3);  // Snow side
        } else if (blockId == 10u) {
            baseCoord = vec2(1, 1);  // Gravel side
        } else if (blockId == 11u) {
            baseCoord = vec2(3, 4);  // Ice side
        } else if (blockId == 12u) {
            baseCoord = vec2(3, 2);  // Alpine Grass side
        } else if (blockId == 13u) {
            baseCoord = vec2(1, 9);  // Alpine Lake side
        } else if (blockId == 14u) {
            baseCoord = vec2(3, 1);  // Clay side
        } else if (blockId == 15u) {
            baseCoord = vec2(0, 1);  // Cobblestone side
        } else if (blockId == 16u) {
            baseCoord = vec2(3, 0);  // Snowy Dirt side (grass side)
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
