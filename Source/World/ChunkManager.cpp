#include "ChunkManager.h"
#include <cmath>
#include <algorithm>
#include "stb_perlin.h"

ChunkManager::ChunkManager(Config& config) 
    : config(config), 
      viewDistanceInChunks(16),  // Default view distance of 16 chunks
      voxelScale(config.voxelScale) {
}

void ChunkManager::init(Biome& biome) {
    this->defaultBiome = &biome;
    // Add the default biome as PLAINS
    biomes[BiomeType::PLAINS] = &biome;
}

void ChunkManager::addBiome(BiomeType type, Biome* biome) {
    if (biome) {
        biomes[type] = biome;
    }
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

// Implementation of new biome control methods
void ChunkManager::setForcedBiome(BiomeType biomeType) {
    forcedBiomeType = biomeType;
    isBiomeForced = true;
}

void ChunkManager::clearForcedBiome() {
    isBiomeForced = false;
}

BiomeType ChunkManager::getCurrentBiomeAt(const glm::vec3& worldPos) const {
    int chunkX, chunkZ;
    worldToChunkCoords(worldPos, chunkX, chunkZ);
    return getBiomeTypeForChunk(chunkX, chunkZ);
}

void ChunkManager::setBiomeBlendFactor(float factor) {
    biomeBlendFactor = std::max(0.0f, std::min(1.0f, factor));
}

void ChunkManager::regenerateChunk(int chunkX, int chunkZ) {
    std::pair<int, int> key = std::make_pair(chunkX, chunkZ);
    auto it = chunks.find(key);
    
    if (it != chunks.end()) {
        // Get the chunk
        auto chunk = it->second;
        
        // Clear the existing data (reinitialize with empty blocks)
        for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
            for (int y = 0; y < Chunk::CHUNK_SIZE_Y; y++) {
                for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
                    chunk->setVoxel(x, y, z, 0); // Set to air
                }
            }
        }
        
        // Regenerate terrain
        generateTerrainForChunk(chunkX, chunkZ);
        
        // Regenerate mesh
        chunk->generateMesh();
    }
}

// Modify the existing getBiomeTypeForChunk method to use forced biome if set
BiomeType ChunkManager::getBiomeTypeForChunk(int chunkX, int chunkZ) const {
    // If a biome is forced, return that instead of calculating
    if (isBiomeForced) {
        return forcedBiomeType;
    }
    
    // Use noise to blend between biomes
    float biomeScale = 0.02f;
    float temperatureNoise = stb_perlin_noise3(chunkX * biomeScale, 0.0f, chunkZ * biomeScale, 0, 0, 0);
    float rainfallNoise = stb_perlin_noise3(chunkX * biomeScale + 100.0f, 0.0f, chunkZ * biomeScale + 100.0f, 0, 0, 0);
    float elevationNoise = stb_perlin_noise3(chunkX * biomeScale - 100.0f, 0.0f, chunkZ * biomeScale - 100.0f, 0, 0, 0);
    
    // Normalize to 0-1 range
    temperatureNoise = (temperatureNoise + 1.0f) * 0.5f;
    rainfallNoise = (rainfallNoise + 1.0f) * 0.5f;
    elevationNoise = (elevationNoise + 1.0f) * 0.5f;
    
    // Temperature and rainfall determine the biome
    // High temperature + low rainfall = DESERT
    if (temperatureNoise > 0.7f && rainfallNoise < 0.3f) {
        return BiomeType::DESERT;
    }
    
    // High elevation = MOUNTAINS
    if (elevationNoise > 0.75f) {
        return BiomeType::MOUNTAINS;
    }
    
    // High rainfall + moderate temperature = FOREST
    if (rainfallNoise > 0.6f && temperatureNoise > 0.4f && temperatureNoise < 0.7f) {
        return BiomeType::FOREST;
    }
    
    // Low temperature = SNOWY_PLAINS
    if (temperatureNoise < 0.3f) {
        return BiomeType::SNOWY_PLAINS;
    }
    
    // Very high rainfall and low elevation = LAKE
    if (rainfallNoise > 0.8f && elevationNoise < 0.3f) {
        return BiomeType::LAKE;
    }
    
    // High elevation + low temperature = ALPINE
    if (elevationNoise > 0.65f && temperatureNoise < 0.4f) {
        return BiomeType::ALPINE;
    }
    
    // Default biome
    return BiomeType::PLAINS;
}

// Add new method to get blended biomes for smoother transitions
std::vector<BiomeBlend> ChunkManager::getBlendedBiomes(int chunkX, int chunkZ) const {
    std::vector<BiomeBlend> results;
    
    // If a biome is forced, only return that biome with full weight
    if (isBiomeForced) {
        results.push_back({forcedBiomeType, 1.0f});
        return results;
    }
    
    // If blend factor is 0, just return the main biome
    if (biomeBlendFactor < 0.01f) {
        results.push_back({getBiomeTypeForChunk(chunkX, chunkZ), 1.0f});
        return results;
    }
    
    // Use a larger area to sample biomes for blending
    const int blendRadius = static_cast<int>(1 + biomeBlendFactor * 2);
    
    // Sample a grid around the target chunk
    std::unordered_map<BiomeType, float> biomeWeights;
    float totalWeight = 0.0f;
    
    for (int dx = -blendRadius; dx <= blendRadius; dx++) {
        for (int dz = -blendRadius; dz <= blendRadius; dz++) {
            float dist = std::sqrt(dx*dx + dz*dz);
            if (dist > blendRadius) continue;
            
            // Weight is inverse of distance (closer biomes have more influence)
            float weight = 1.0f - (dist / blendRadius);
            weight = weight * weight; // Square for stronger central bias
            
            BiomeType type = getBiomeTypeForChunk(chunkX + dx, chunkZ + dz);
            biomeWeights[type] += weight;
            totalWeight += weight;
        }
    }
    
    // Normalize weights and create the blend list
    for (const auto& pair : biomeWeights) {
        if (totalWeight > 0) {
            float normWeight = pair.second / totalWeight;
            // Only include biomes with significant influence
            if (normWeight > 0.1f) {
                results.push_back({pair.first, normWeight});
            }
        }
    }
    
    // Sort by weight descending
    std::sort(results.begin(), results.end(), 
              [](const BiomeBlend& a, const BiomeBlend& b) {
                  return a.weight > b.weight;
              });
    
    // If somehow empty, add the direct biome
    if (results.empty()) {
        results.push_back({getBiomeTypeForChunk(chunkX, chunkZ), 1.0f});
    }
    
    return results;
}

// Modify generateHeightmapForChunk to use blended biomes for smoother transitions
void ChunkManager::generateHeightmapForChunk(int chunkX, int chunkZ, std::vector<std::vector<int>>& heightMap) const {
    // Get blended biomes for this chunk
    std::vector<BiomeBlend> biomeBlends = getBlendedBiomes(chunkX, chunkZ);
    
    // Get primary biome (highest weight)
    BiomeType primaryBiomeType = biomeBlends[0].type;
    Biome* primaryBiome = nullptr;
    
    // Use find() to get the biome since we're in a const method
    auto primaryBiomeIter = biomes.find(primaryBiomeType);
    if (primaryBiomeIter != biomes.end()) {
        primaryBiome = primaryBiomeIter->second;
    } else {
        primaryBiome = defaultBiome;
    }
    
    // Ensure heightmap has proper dimensions
    heightMap.resize(Chunk::CHUNK_SIZE_X);
    for (auto& col : heightMap) {
        col.resize(Chunk::CHUNK_SIZE_Z);
    }
    
    // Use Perlin noise for height generation with biome-specific parameters
    float baseFrequency = 0.01f;
    float baseAmplitude = 20.0f;
    int baseWaterLevel = primaryBiome->getWaterLevel();
    
    // Get references to commonly used biomes
    auto mountainBiomeIter = biomes.find(BiomeType::MOUNTAINS);
    auto desertBiomeIter = biomes.find(BiomeType::DESERT);
    auto lakeBiomeIter = biomes.find(BiomeType::LAKE);
    auto forestBiomeIter = biomes.find(BiomeType::FOREST);
    auto snowyBiomeIter = biomes.find(BiomeType::SNOWY_PLAINS);
    auto alpineBiomeIter = biomes.find(BiomeType::ALPINE);
    
    // Apply biome-specific base parameters for primary biome
    if (mountainBiomeIter != biomes.end() && primaryBiome == mountainBiomeIter->second) {
        baseFrequency = 0.015f;
        baseAmplitude = 32.0f;
    } else if (desertBiomeIter != biomes.end() && primaryBiome == desertBiomeIter->second) {
        baseFrequency = 0.008f;
        baseAmplitude = 16.0f;
    } else if (lakeBiomeIter != biomes.end() && primaryBiome == lakeBiomeIter->second) {
        baseAmplitude = 14.0f;
    } else if (forestBiomeIter != biomes.end() && primaryBiome == forestBiomeIter->second) {
        baseFrequency = 0.012f;
        baseAmplitude = 24.0f;
    }
    
    // Generate heightmap
    for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
        for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
            // Convert local coordinates to world coordinates for noise calculation
            float worldX = (chunkX * Chunk::CHUNK_SIZE_X + x);
            float worldZ = (chunkZ * Chunk::CHUNK_SIZE_Z + z);
            
            // Generate base height using multiple octaves of noise for more natural terrain
            float heightValue = 0.0f;
            
            // Add multiple octaves of noise
            heightValue += stb_perlin_noise3(worldX * baseFrequency, 0.0f, worldZ * baseFrequency, 0, 0, 0);
            heightValue += 0.5f * stb_perlin_noise3(worldX * baseFrequency * 2.0f, 0.0f, worldZ * baseFrequency * 2.0f, 0, 0, 0);
            heightValue += 0.25f * stb_perlin_noise3(worldX * baseFrequency * 4.0f, 0.0f, worldZ * baseFrequency * 4.0f, 0, 0, 0);
            
            // Normalize to -1 to 1 range
            heightValue /= 1.75f;
            
            // For each additional biome, blend their height contributions
            float blendedHeight = heightValue * baseAmplitude + baseWaterLevel;
            
            // Apply biome-specific height modifications based on weights
            for (const auto& blend : biomeBlends) {
                // Skip the first one as we've already used it for the base
                if (blend.type == primaryBiomeType) continue;
                
                // Only process significant contributors
                if (blend.weight < 0.1f) continue;
                
                // Get the biome
                auto biomeIter = biomes.find(blend.type);
                if (biomeIter == biomes.end()) continue;
                
                Biome* blendBiome = biomeIter->second;
                float blendFrequency = baseFrequency;
                float blendAmplitude = baseAmplitude;
                
                // Determine specific parameters for this biome
                if (blend.type == BiomeType::MOUNTAINS) {
                    // Mountains have more dramatic peaks
                    float mountainNoise = stb_perlin_noise3(worldX * 0.015f, 0.0f, worldZ * 0.015f, 0, 0, 0);
                    if (mountainNoise > 0.3f) {
                        // Add some peaks
                        float peakNoise = stb_perlin_noise3(worldX * 0.03f, 0.0f, worldZ * 0.03f, 0, 0, 0);
                        blendedHeight += blend.weight * ((peakNoise + 0.5f) * 15.0f);
                    }
                } 
                else if (blend.type == BiomeType::DESERT) {
                    // Deserts have gentle rolling dunes
                    float duneNoise = stb_perlin_noise3(worldX * 0.02f, 0.0f, worldZ * 0.02f, 0, 0, 0);
                    blendedHeight += blend.weight * (duneNoise * 4.0f - 2.0f); // Slightly lower overall
                }
                else if (blend.type == BiomeType::FOREST) {
                    // Forests have more varied terrain
                    float forestNoise = stb_perlin_noise3(worldX * 0.025f, 0.0f, worldZ * 0.025f, 0, 0, 0);
                    blendedHeight += blend.weight * (forestNoise * 3.0f);
                }
                else if (blend.type == BiomeType::SNOWY_PLAINS) {
                    // Snowy plains are flatter with occasional hills
                    float snowNoise = stb_perlin_noise3(worldX * 0.01f, 0.0f, worldZ * 0.01f, 0, 0, 0);
                    blendedHeight += blend.weight * (snowNoise * 2.0f - 1.0f); // Generally lower
                }
            }
            
            // Apply final modifications and clamp
            int height = static_cast<int>(blendedHeight);
            
            // Ensure height is within valid range
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
    
    // Get the appropriate biome for this chunk
    BiomeType biomeType = getBiomeTypeForChunk(chunkX, chunkZ);
    Biome* biomeTouse = biomes.count(biomeType) > 0 ? biomes.at(biomeType) : defaultBiome;
    
    // Generate heightmap for this chunk
    std::vector<std::vector<int>> heightMap;
    generateHeightmapForChunk(chunkX, chunkZ, heightMap);
    
    // Get water level from the biome
    int waterLevel = biomeTouse->getWaterLevel();
    
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
            
            // Fill terrain from bottom to height using biome-specific block types
            for (int y = 0; y <= height; y++) {
                unsigned int blockId;
                
                // Select block types based on biome
                if (y == height) {
                    // Top layer - biome specific
                    if (biomeType == BiomeType::DESERT) {
                        blockId = 8; // Sand
                    } else if (biomeType == BiomeType::SNOWY_PLAINS) {
                        blockId = 9; // Snow
                    } else if (biomeType == BiomeType::MOUNTAINS && height > waterLevel + 20) {
                        blockId = 9; // Snow on high mountains
                    } else if (biomeType == BiomeType::FOREST) {
                        blockId = 1; // Grass
                    } else if (biomeType == BiomeType::ALPINE) {
                        blockId = 12; // Alpine grass
                    } else if (height <= waterLevel && isNearWater) {
                        blockId = 8; // Sand near water
                    } else {
                        blockId = 1; // Default grass
                    }
                } else if (y > height - 3) {
                    // Middle layer - based on biome
                    if (biomeType == BiomeType::DESERT) {
                        blockId = 8; // Sand
                    } else if (biomeType == BiomeType::SNOWY_PLAINS) {
                        blockId = 16; // Snowy dirt
                    } else if (biomeType == BiomeType::MOUNTAINS && height > waterLevel + 15) {
                        blockId = 3; // Stone for mountains
                    } else {
                        blockId = 2; // Dirt
                    }
                } else {
                    // Bottom layer
                    if (biomeType == BiomeType::MOUNTAINS) {
                        blockId = 3; // Stone for mountains
                    } else if (biomeType == BiomeType::DESERT) {
                        blockId = 14; // Clay under sand
                    } else if (y < 5) {
                        blockId = 15; // Cobblestone at very bottom
                    } else {
                        blockId = 3; // Stone
                    }
                }
                
                chunk->setVoxel(x, y, z, blockId);
            }
            
            // Fill water - different types based on biome
            for (int y = height + 1; y <= waterLevel; y++) {
                if (height < waterLevel) {
                    if (biomeType == BiomeType::ALPINE) {
                        chunk->setVoxel(x, y, z, 13); // Alpine lake water
                    } else if (biomeType == BiomeType::SNOWY_PLAINS) {
                        chunk->setVoxel(x, y, z, 11); // Ice
                    } else {
                        chunk->setVoxel(x, y, z, 7); // Regular water
                    }
                }
            }
        }
    }
    
    // Add biome-specific features
    if (biomeTouse) {
        // Tree placement depends on biome
        float treeDensity = 0.01f; // Default
        
        // Adjust tree density by biome
        if (biomeType == BiomeType::FOREST) {
            treeDensity = 0.04f; // More trees in forest
        } else if (biomeType == BiomeType::DESERT) {
            treeDensity = 0.002f; // Very few trees in desert
        } else if (biomeType == BiomeType::SNOWY_PLAINS) {
            treeDensity = 0.008f; // Few trees in snowy plains
        } else if (biomeType == BiomeType::MOUNTAINS) {
            treeDensity = 0.015f; // Moderate trees in mountains
        }
        
        for (int x = 2; x < Chunk::CHUNK_SIZE_X - 2; ++x) {
            for (int z = 2; z < Chunk::CHUNK_SIZE_Z; ++z) {
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
                        // Place different tree types based on biome
                        if (biomeType == BiomeType::DESERT) {
                            // Desert - place a cactus (simple 3-block tall structure)
                            for (int y = 1; y <= 2; y++) {
                                chunk->setVoxel(x, height + y, z, 4); // Reuse wood for cactus
                            }
                        } 
                        else {
                            // Standard tree for other biomes
                            // Place tree trunk (3 blocks tall)
                            for (int y = 1; y <= 3; y++) {
                                chunk->setVoxel(x, height + y, z, 4); // Wood
                            }
                            
                            // Place leaves appropriate for the biome
                            unsigned int leafBlockId = 5; // Default leaves
                            if (biomeType == BiomeType::SNOWY_PLAINS || biomeType == BiomeType::MOUNTAINS && height > waterLevel + 20) {
                                // Snowy trees have fewer leaves
                                for (int lx = -1; lx <= 1; lx++) {
                                    for (int ly = 3; ly <= 4; ly++) {
                                        for (int lz = -1; lz <= 1; lz++) {
                                            // Make a smaller shape for snowy trees
                                            if (lx*lx + (ly-3)*(ly-3) + lz*lz > 2) continue;
                                            
                                            // Don't replace existing trunk blocks
                                            if (lx == 0 && lz == 0) continue;
                                            
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
                            else {
                                // Regular or dense trees
                                int leafSize = (biomeType == BiomeType::FOREST) ? 3 : 2;
                                
                                for (int lx = -leafSize; lx <= leafSize; lx++) {
                                    for (int ly = 3; ly <= 5; ly++) {
                                        for (int lz = -leafSize; lz <= leafSize; lz++) {
                                            // Skip if too far (make a rough sphere)
                                            if (lx*lx + (ly-4)*(ly-4) + lz*lz > (biomeType == BiomeType::FOREST ? 7 : 5)) continue;
                                            
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
    }
}