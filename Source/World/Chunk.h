#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <array>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Voxel.h"

// Forward declarations
class ChunkManager;

class Chunk {
public:
    // Constants for chunk dimensions
    static const int CHUNK_SIZE_X = 16;
    static const int CHUNK_SIZE_Y = 64;  // Height can be higher
    static const int CHUNK_SIZE_Z = 16;

    // Constructor - takes chunk coordinates (in chunk space, not world space)
    Chunk(int chunkX, int chunkZ, float voxelScale);
    ~Chunk() = default;

    // Get chunk coordinates
    int getChunkX() const { return chunkX; }
    int getChunkZ() const { return chunkZ; }
    
    // Convert local coordinates to world coordinates
    glm::vec3 toWorldPosition(int localX, int localY, int localZ) const;
    
    // Convert world coordinates to local coordinates
    bool toLocalPosition(const glm::vec3& worldPos, int& localX, int& localY, int& localZ) const;
    
    // Check if world position is within this chunk
    bool containsWorldPosition(const glm::vec3& worldPos) const;

    // Check if chunk coordinates are within bounds
    bool isValidLocalPosition(int localX, int localY, int localZ) const;

    // Voxel operations
    void setVoxel(int localX, int localY, int localZ, unsigned int blockId);
    unsigned int getVoxelBlockId(int localX, int localY, int localZ) const;
    bool isVoxelSolid(int localX, int localY, int localZ) const;
    
    // Generate mesh for this chunk
    std::vector<Voxel> generateMesh();
    
    // Check if chunk needs remeshing after block changes
    bool needsRemesh() const { return isDirty; }
    void markDirty() { isDirty = true; }
    
    // Get reference to voxel data
    const std::vector<Voxel>& getVoxels() const { return chunkMesh; }
    
    // Return true if chunk has any visible voxels
    bool hasVisibleVoxels() const { return !chunkMesh.empty(); }

private:
    // Chunk coordinates (in chunk space)
    int chunkX, chunkZ;
    
    // Scale factor for voxels
    float voxelScale;
    
    // 3D array of block IDs (0 = air/empty)
    std::array<unsigned int, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z> blocks;
    
    // Generated mesh for rendering
    std::vector<Voxel> chunkMesh;
    
    // Flag indicating if mesh needs to be regenerated
    bool isDirty;
    
    // Helper methods
    int getBlockIndex(int localX, int localY, int localZ) const;
    bool isFaceVisible(int localX, int localY, int localZ, int dx, int dy, int dz) const;
};

#endif // CHUNK_H