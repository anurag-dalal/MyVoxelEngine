#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "../Utils/ConfigReader.h"
#include "Voxel.h"
#include "Generation/Biome.h"

// Hash function for chunk coordinates
struct ChunkCoordHash {
    std::size_t operator()(const std::pair<int, int>& coord) const {
        return std::hash<int>()(coord.first) ^ (std::hash<int>()(coord.second) << 1);
    }
};

class ChunkManager {
public:
    ChunkManager(Config& config);
    ~ChunkManager() = default;
    
    // Initialize the chunk manager with a biome for terrain generation
    void init(Biome& biome);
    
    // Chunk operations
    void loadChunk(int chunkX, int chunkZ);
    void unloadChunk(int chunkX, int chunkZ);
    bool isChunkLoaded(int chunkX, int chunkZ) const;
    
    // Get chunk by coordinates
    std::shared_ptr<Chunk> getChunk(int chunkX, int chunkZ) const;
    
    // Convert world position to chunk coordinates
    void worldToChunkCoords(const glm::vec3& worldPos, int& chunkX, int& chunkZ) const;
    
    // Get chunk at world position
    std::shared_ptr<Chunk> getChunkAtPosition(const glm::vec3& worldPos) const;
    
    // Voxel operations that work across chunks
    unsigned int getVoxelBlockId(const glm::vec3& worldPos) const;
    void setVoxel(const glm::vec3& worldPos, unsigned int blockId);
    bool isVoxelSolid(const glm::vec3& worldPos) const;
    
    // Update chunk loading based on camera position
    void updateChunks(const glm::vec3& cameraPos);
    
    // Get mesh for rendering
    std::vector<Voxel> getVisibleVoxels() const;
    
    // Terrain generation methods
    void generateTerrainForChunk(int chunkX, int chunkZ);

private:
    // Map of loaded chunks
    std::unordered_map<std::pair<int, int>, std::shared_ptr<Chunk>, ChunkCoordHash> chunks;
    
    // Config reference
    Config& config;
    
    // View distance in chunks
    int viewDistanceInChunks;
    
    // Voxel scale
    float voxelScale;
    
    // Biome reference for terrain generation
    Biome* biome = nullptr;
    
    // Helper methods
    void updateChunkMeshes();
    void generateHeightmapForChunk(int chunkX, int chunkZ, std::vector<std::vector<int>>& heightMap) const;
};

#endif // CHUNK_MANAGER_H