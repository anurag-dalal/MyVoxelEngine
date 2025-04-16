#include "Chunk.h"
#include <iostream>

Chunk::Chunk(int chunkX, int chunkZ, float voxelScale)
    : chunkX(chunkX), chunkZ(chunkZ), voxelScale(voxelScale), isDirty(true) {
    // Initialize all blocks to air (0)
    blocks.fill(0);
}

glm::vec3 Chunk::toWorldPosition(int localX, int localY, int localZ) const {
    // Convert local coordinates to world coordinates
    float worldX = (chunkX * CHUNK_SIZE_X + localX) * voxelScale;
    float worldY = localY * voxelScale;
    float worldZ = (chunkZ * CHUNK_SIZE_Z + localZ) * voxelScale;
    
    return glm::vec3(worldX, worldY, worldZ);
}

bool Chunk::toLocalPosition(const glm::vec3& worldPos, int& localX, int& localY, int& localZ) const {
    // Convert world position to chunk-local coordinates
    // First, convert to voxel coordinates (integer)
    int voxelX = static_cast<int>(worldPos.x / voxelScale);
    int voxelY = static_cast<int>(worldPos.y / voxelScale);
    int voxelZ = static_cast<int>(worldPos.z / voxelScale);
    
    // Then, convert to local coordinates
    localX = voxelX - chunkX * CHUNK_SIZE_X;
    localY = voxelY;
    localZ = voxelZ - chunkZ * CHUNK_SIZE_Z;
    
    // Check if the coordinates are within this chunk
    return isValidLocalPosition(localX, localY, localZ);
}

bool Chunk::containsWorldPosition(const glm::vec3& worldPos) const {
    int localX, localY, localZ;
    return toLocalPosition(worldPos, localX, localY, localZ);
}

bool Chunk::isValidLocalPosition(int localX, int localY, int localZ) const {
    return localX >= 0 && localX < CHUNK_SIZE_X && 
           localY >= 0 && localY < CHUNK_SIZE_Y && 
           localZ >= 0 && localZ < CHUNK_SIZE_Z;
}

int Chunk::getBlockIndex(int localX, int localY, int localZ) const {
    return localX + 
           localY * CHUNK_SIZE_X + 
           localZ * CHUNK_SIZE_X * CHUNK_SIZE_Y;
}

void Chunk::setVoxel(int localX, int localY, int localZ, unsigned int blockId) {
    if (isValidLocalPosition(localX, localY, localZ)) {
        int index = getBlockIndex(localX, localY, localZ);
        blocks[index] = blockId;
        isDirty = true;
    }
}

unsigned int Chunk::getVoxelBlockId(int localX, int localY, int localZ) const {
    if (isValidLocalPosition(localX, localY, localZ)) {
        int index = getBlockIndex(localX, localY, localZ);
        return blocks[index];
    }
    return 0; // Return air for invalid positions
}

bool Chunk::isVoxelSolid(int localX, int localY, int localZ) const {
    // Air (blockId = 0) is not solid
    // Water (blockId = 7) is semi-transparent, but we'll consider it non-solid for face culling
    unsigned int blockId = getVoxelBlockId(localX, localY, localZ);
    return blockId != 0 && blockId != 7;
}

bool Chunk::isFaceVisible(int localX, int localY, int localZ, int dx, int dy, int dz) const {
    // Check if neighboring voxel exists and is non-solid
    int nx = localX + dx;
    int ny = localY + dy;
    int nz = localZ + dz;
    
    // If neighboring position is outside chunk bounds, consider the face visible
    if (!isValidLocalPosition(nx, ny, nz)) {
        return true;
    }
    
    // Check if the neighboring voxel is non-solid (air or water)
    return !isVoxelSolid(nx, ny, nz);
}

std::vector<Voxel> Chunk::generateMesh() {
    // Clear the previous mesh
    chunkMesh.clear();
    
    // Iterate through all blocks in the chunk
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
        for (int y = 0; y < CHUNK_SIZE_Y; y++) {
            for (int z = 0; z < CHUNK_SIZE_Z; z++) {
                unsigned int blockId = getVoxelBlockId(x, y, z);
                
                // Skip air blocks
                if (blockId == 0) {
                    continue;
                }
                
                // Calculate world position for this voxel
                glm::vec3 position = toWorldPosition(x, y, z);
                
                // Only add visible faces (basic culling)
                // We only add a voxel if at least one of its faces is visible
                bool hasVisibleFace = 
                    isFaceVisible(x, y, z, 0, 0, -1) || // Back face (-Z)
                    isFaceVisible(x, y, z, 0, 0, 1) ||  // Front face (+Z)
                    isFaceVisible(x, y, z, -1, 0, 0) || // Left face (-X)
                    isFaceVisible(x, y, z, 1, 0, 0) ||  // Right face (+X)
                    isFaceVisible(x, y, z, 0, -1, 0) || // Bottom face (-Y)
                    isFaceVisible(x, y, z, 0, 1, 0);    // Top face (+Y)
                
                if (hasVisibleFace) {
                    chunkMesh.emplace_back(position, blockId);
                }
            }
        }
    }
    
    // Mark chunk as up-to-date
    isDirty = false;
    
    return chunkMesh;
}