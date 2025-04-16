#include "ChunkManager.h"
#include <cmath>
#include <algorithm>
#include "stb_perlin.h"

ChunkManager::ChunkManager(Config& config) 
    : config(config), 
      viewDistanceInChunks(16),  // Default view distance of 8 chunks
      voxelScale(config.voxelScale) {
}

void ChunkManager::init(Biome& biome) {
    this->biome = &biome;
}

void ChunkManager::loadChunk(int chunkX, int chunkZ) {
    std::pair<int, int> key = std::make_pair(chunkX, chunkZ);
    if (chunks.find(key) == chunks.end()) {
        // Create a new chunk
        auto chunk = std::make_shared<Chunk>(chunkX, chunkZ, voxelScale);
        chunks[key] = chunk;
        
        // Generate terrain for this chunk
        generateTerrainForChunk(chunkX, chunkZ);
        
        // Generate mesh
        chunk->generateMesh();
    }
}

void ChunkManager::unloadChunk(int chunkX, int chunkZ) {
    std::pair<int, int> key = std::make_pair(chunkX, chunkZ);
    chunks.erase(key);
}

bool ChunkManager::isChunkLoaded(int chunkX, int chunkZ) const {
    std::pair<int, int> key = std::make_pair(chunkX, chunkZ);
    return chunks.find(key) != chunks.end();
}

std::shared_ptr<Chunk> ChunkManager::getChunk(int chunkX, int chunkZ) const {
    std::pair<int, int> key = std::make_pair(chunkX, chunkZ);
    auto it = chunks.find(key);
    if (it != chunks.end()) {
        return it->second;
    }
    return nullptr;
}

void ChunkManager::worldToChunkCoords(const glm::vec3& worldPos, int& chunkX, int& chunkZ) const {
    // Convert world position to chunk coordinates
    float voxelX = worldPos.x / voxelScale;
    float voxelZ = worldPos.z / voxelScale;
    
    chunkX = static_cast<int>(std::floor(voxelX / Chunk::CHUNK_SIZE_X));
    chunkZ = static_cast<int>(std::floor(voxelZ / Chunk::CHUNK_SIZE_Z));
}

std::shared_ptr<Chunk> ChunkManager::getChunkAtPosition(const glm::vec3& worldPos) const {
    int chunkX, chunkZ;
    worldToChunkCoords(worldPos, chunkX, chunkZ);
    return getChunk(chunkX, chunkZ);
}

unsigned int ChunkManager::getVoxelBlockId(const glm::vec3& worldPos) const {
    auto chunk = getChunkAtPosition(worldPos);
    if (!chunk) {
        return 0; // Air if chunk not loaded
    }
    
    // Convert world position to local chunk coordinates
    int localX, localY, localZ;
    if (chunk->toLocalPosition(worldPos, localX, localY, localZ)) {
        return chunk->getVoxelBlockId(localX, localY, localZ);
    }
    
    return 0; // Air if position is invalid
}

void ChunkManager::setVoxel(const glm::vec3& worldPos, unsigned int blockId) {
    auto chunk = getChunkAtPosition(worldPos);
    if (!chunk) {
        return;
    }
    
    // Convert world position to local chunk coordinates
    int localX, localY, localZ;
    if (chunk->toLocalPosition(worldPos, localX, localY, localZ)) {
        chunk->setVoxel(localX, localY, localZ, blockId);
        
        // Mark neighboring chunks as dirty if the voxel is on the edge
        if (localX == 0) {
            auto neighborChunk = getChunk(chunk->getChunkX() - 1, chunk->getChunkZ());
            if (neighborChunk) {
                neighborChunk->markDirty();
            }
        }
        else if (localX == Chunk::CHUNK_SIZE_X - 1) {
            auto neighborChunk = getChunk(chunk->getChunkX() + 1, chunk->getChunkZ());
            if (neighborChunk) {
                neighborChunk->markDirty();
            }
        }
        
        if (localZ == 0) {
            auto neighborChunk = getChunk(chunk->getChunkX(), chunk->getChunkZ() - 1);
            if (neighborChunk) {
                neighborChunk->markDirty();
            }
        }
        else if (localZ == Chunk::CHUNK_SIZE_Z - 1) {
            auto neighborChunk = getChunk(chunk->getChunkX(), chunk->getChunkZ() + 1);
            if (neighborChunk) {
                neighborChunk->markDirty();
            }
        }
    }
}

bool ChunkManager::isVoxelSolid(const glm::vec3& worldPos) const {
    unsigned int blockId = getVoxelBlockId(worldPos);
    return blockId != 0 && blockId != 7; // Air and water are not solid
}

void ChunkManager::updateChunks(const glm::vec3& cameraPos) {
    // Convert camera position to chunk coordinates
    int centerChunkX, centerChunkZ;
    worldToChunkCoords(cameraPos, centerChunkX, centerChunkZ);
    
    // Determine the range of chunks to load
    int minChunkX = centerChunkX - viewDistanceInChunks;
    int maxChunkX = centerChunkX + viewDistanceInChunks;
    int minChunkZ = centerChunkZ - viewDistanceInChunks;
    int maxChunkZ = centerChunkZ + viewDistanceInChunks;
    
    // Load chunks in range
    for (int x = minChunkX; x <= maxChunkX; x++) {
        for (int z = minChunkZ; z <= maxChunkZ; z++) {
            if (!isChunkLoaded(x, z)) {
                loadChunk(x, z);
            }
        }
    }
    
    // Unload chunks outside of view distance
    std::vector<std::pair<int, int>> chunksToUnload;
    
    for (const auto& [coords, chunk] : chunks) {
        int chunkX = coords.first;
        int chunkZ = coords.second;
        
        if (chunkX < minChunkX || chunkX > maxChunkX || chunkZ < minChunkZ || chunkZ > maxChunkZ) {
            chunksToUnload.push_back(coords);
        }
    }
    
    for (const auto& coords : chunksToUnload) {
        unloadChunk(coords.first, coords.second);
    }
    
    // Update meshes for dirty chunks
    updateChunkMeshes();
}

void ChunkManager::updateChunkMeshes() {
    for (auto& [coords, chunk] : chunks) {
        if (chunk->needsRemesh()) {
            chunk->generateMesh();
        }
    }
}

std::vector<Voxel> ChunkManager::getVisibleVoxels() const {
    std::vector<Voxel> visibleVoxels;
    
    for (const auto& [coords, chunk] : chunks) {
        if (chunk->hasVisibleVoxels()) {
            const auto& chunkVoxels = chunk->getVoxels();
            visibleVoxels.insert(visibleVoxels.end(), chunkVoxels.begin(), chunkVoxels.end());
        }
    }
    
    return visibleVoxels;
}

void ChunkManager::generateHeightmapForChunk(int chunkX, int chunkZ, std::vector<std::vector<int>>& heightMap) const {
    // Ensure heightmap has proper dimensions
    heightMap.resize(Chunk::CHUNK_SIZE_X);
    for (auto& col : heightMap) {
        col.resize(Chunk::CHUNK_SIZE_Z);
    }
    
    // Use Perlin noise for height generation
    float frequency = 0.01f; // Adjust these parameters to control terrain
    float amplitude = 20.0f;
    int waterLevel = 12; // Default water level
    
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
            // Convert local coordinates to world coordinates for noise calculation
            float worldX = (chunkX * Chunk::CHUNK_SIZE_X + x) * frequency;
            float worldZ = (chunkZ * Chunk::CHUNK_SIZE_Z + z) * frequency;
            
            // Generate base noise value
            float noiseValue = stb_perlin_noise3(worldX, 0.0f, worldZ, 0, 0, 0);
            
            // Apply amplitude and add base height
            int height = static_cast<int>(noiseValue * amplitude + waterLevel + 5);
            
            // Clamp height to valid range
            height = std::max(1, std::min(height, Chunk::CHUNK_SIZE_Y - 1));
            
            heightMap[x][z] = height;
        }
    }
}

void ChunkManager::generateTerrainForChunk(int chunkX, int chunkZ) {
    auto chunk = getChunk(chunkX, chunkZ);
    if (!chunk) {
        return;
    }
    
    // Generate heightmap for this chunk
    std::vector<std::vector<int>> heightMap;
    generateHeightmapForChunk(chunkX, chunkZ, heightMap);
    
    // Water level
    int waterLevel = 12;
    
    // Generate terrain based on heightmap
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
            int height = heightMap[x][z];
            
            // Check if voxel has neighbors with water
            bool isNearWater = false;
            const int dx[] = {-1, 1, 0, 0};
            const int dz[] = {0, 0, -1, 1};
            
            for (int i = 0; i < 4; i++) {
                int nx = x + dx[i];
                int nz = z + dz[i];
                
                if (nx >= 0 && nx < Chunk::CHUNK_SIZE_X && nz >= 0 && nz < Chunk::CHUNK_SIZE_Z) {
                    if (heightMap[nx][nz] < waterLevel) {
                        isNearWater = true;
                        break;
                    }
                } else {
                    // Check neighboring chunks
                    int neighborChunkX = chunkX;
                    int neighborChunkZ = chunkZ;
                    int neighborX = nx;
                    int neighborZ = nz;
                    
                    if (nx < 0) {
                        neighborChunkX--;
                        neighborX = Chunk::CHUNK_SIZE_X + nx;
                    } else if (nx >= Chunk::CHUNK_SIZE_X) {
                        neighborChunkX++;
                        neighborX = nx - Chunk::CHUNK_SIZE_X;
                    }
                    
                    if (nz < 0) {
                        neighborChunkZ--;
                        neighborZ = Chunk::CHUNK_SIZE_Z + nz;
                    } else if (nz >= Chunk::CHUNK_SIZE_Z) {
                        neighborChunkZ++;
                        neighborZ = nz - Chunk::CHUNK_SIZE_Z;
                    }
                    
                    auto neighborChunk = getChunk(neighborChunkX, neighborChunkZ);
                    if (neighborChunk) {
                        // Get height from neighboring chunk if available
                        // We'll skip this for simplicity, assuming chunk borders won't affect the water check much
                    }
                }
            }
            
            // Fill terrain from bottom to height
            for (int y = 0; y <= height; y++) {
                unsigned int blockId;
                
                if (y == height) {
                    if (height <= waterLevel && isNearWater) {
                        blockId = 8; // Sand near water
                    } else {
                        blockId = 1; // Grass on top
                    }
                } else if (y > height - 3) {
                    blockId = 2; // Dirt layer
                } else {
                    blockId = 3; // Stone beneath
                }
                
                chunk->setVoxel(x, y, z, blockId);
            }
            
            // Fill water
            for (int y = height + 1; y <= waterLevel; y++) {
                if (height < waterLevel) {
                    chunk->setVoxel(x, y, z, 7); // Water
                }
            }
        }
    }
    
    // Random tree placement (if biome is provided)
    if (biome) {
        float treeDensity = 0.01f;
        
        for (int x = 2; x < Chunk::CHUNK_SIZE_X - 2; ++x) {
            for (int z = 2; z < Chunk::CHUNK_SIZE_Z - 2; ++z) {
                if ((static_cast<float>(rand()) / RAND_MAX) < treeDensity) {
                    int height = heightMap[x][z];
                    
                    if (height <= waterLevel) continue;
                    
                    // Basic check for tree placement
                    bool canPlaceTree = true;
                    for (int dx = -1; dx <= 1 && canPlaceTree; ++dx) {
                        for (int dz = -1; dz <= 1 && canPlaceTree; ++dz) {
                            int nx = x + dx;
                            int nz = z + dz;
                            
                            if (nx >= 0 && nx < Chunk::CHUNK_SIZE_X && nz >= 0 && nz < Chunk::CHUNK_SIZE_Z) {
                                if (abs(heightMap[nx][nz] - height) > 2) {
                                    canPlaceTree = false;
                                }
                            }
                        }
                    }
                    
                    if (canPlaceTree) {
                        // Place tree trunk (3 blocks tall)
                        for (int y = 1; y <= 3; y++) {
                            chunk->setVoxel(x, height + y, z, 4); // Wood
                        }
                        
                        // Place leaves
                        for (int lx = -2; lx <= 2; lx++) {
                            for (int ly = 3; ly <= 5; ly++) {
                                for (int lz = -2; lz <= 2; lz++) {
                                    // Skip if too far (make a rough sphere)
                                    if (lx*lx + (ly-4)*(ly-4) + lz*lz > 5) continue;
                                    
                                    // Don't replace existing trunk blocks
                                    if (lx == 0 && lz == 0 && ly < 4) continue;
                                    
                                    // Check if we're still in this chunk
                                    int leafX = x + lx;
                                    int leafY = height + ly;
                                    int leafZ = z + lz;
                                    
                                    if (chunk->isValidLocalPosition(leafX, leafY, leafZ)) {
                                        chunk->setVoxel(leafX, leafY, leafZ, 5); // Leaves
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}