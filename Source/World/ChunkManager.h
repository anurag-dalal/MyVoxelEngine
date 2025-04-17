#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <unordered_map>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Chunk.h"
#include "../Utils/ConfigReader.h"
#include "Voxel.h"
#include "Generation/Biomes/Biome.h"

// Hash function for chunk coordinates
struct ChunkCoordHash {
    std::size_t operator()(const std::pair<int, int>& coord) const {
        return std::hash<int>()(coord.first) ^ (std::hash<int>()(coord.second) << 1);
    }
};

// Enum for biome types
enum class BiomeType {
    PLAINS,
    FOREST,
    DESERT,
    MOUNTAINS,
    SNOWY_PLAINS,
    LAKE,
    ALPINE
};

// Struct for biome blending weight
struct BiomeBlend {
    BiomeType type;
    float weight; // 0.0 to 1.0
};

class ChunkManager {
public:
    ChunkManager(Config& config);
    ~ChunkManager() = default;
    
    // Initialize the chunk manager with biomes for terrain generation
    void init(Biome& defaultBiome);
    void addBiome(BiomeType type, Biome* biome);
    
    // Biome control methods
    void setForcedBiome(BiomeType biomeType);
    void clearForcedBiome();
    BiomeType getCurrentBiomeAt(const glm::vec3& worldPos) const;
    void setBiomeBlendFactor(float factor); // 0.0 = sharp transitions, 1.0 = maximum blending
    
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
    
    // Regenerate terrain for a chunk with a specific biome
    void regenerateChunk(int chunkX, int chunkZ);

private:
    // Map of loaded chunks
    std::unordered_map<std::pair<int, int>, std::shared_ptr<Chunk>, ChunkCoordHash> chunks;
    
    // Config reference
    Config& config;
    
    // View distance in chunks
    int viewDistanceInChunks;
    
    // Voxel scale
    float voxelScale;
    
    // Biome references for terrain generation
    std::unordered_map<BiomeType, Biome*> biomes;
    Biome* defaultBiome = nullptr;
    
    // Biome control
    bool isBiomeForced = false;
    BiomeType forcedBiomeType = BiomeType::PLAINS;
    float biomeBlendFactor = 0.5f;
    
    // Helper methods
    void updateChunkMeshes();
    void generateHeightmapForChunk(int chunkX, int chunkZ, std::vector<std::vector<int>>& heightMap) const;
    BiomeType getBiomeTypeForChunk(int chunkX, int chunkZ) const;
    std::vector<BiomeBlend> getBlendedBiomes(int chunkX, int chunkZ) const;
};

#endif // CHUNK_MANAGER_H