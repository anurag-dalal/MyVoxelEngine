#include "BasicBiome.h"
#include "stb_perlin.h"

BasicBiome::BasicBiome(const Config& config) 
    : Biome(config)
{}

std::vector<std::vector<int>> BasicBiome::generateHeightMap() {
    const int vox_width = config.gridConfig.vox_width;
    const int vox_depth = config.gridConfig.vox_depth;
    
    std::vector<std::vector<int>> terrainHeights(vox_width, std::vector<int>(vox_depth));
    
    for (int x = 0; x < vox_width; ++x) {
        for (int z = 0; z < vox_depth; ++z) {
            float noiseValue = stb_perlin_noise3(x * frequency, 0.0f, z * frequency, 0, 0, 0);
            noiseValue = (noiseValue + 1.0f) / 2.0f;
            terrainHeights[x][z] = static_cast<int>(noiseValue * amplitude);
        }
    }
    
    return terrainHeights;
}

void BasicBiome::placeTrees(std::vector<Voxel>& voxels, const std::vector<std::vector<int>>& heights) {
    const int vox_width = config.gridConfig.vox_width;
    const int vox_depth = config.gridConfig.vox_depth;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> treeDist(0.0, 1.0);
    float treeDensity = 0.01f;

    for (int x = 2; x < vox_width - 2; ++x) {
        for (int z = 2; z < vox_depth - 2; ++z) {
            if (treeDist(gen) < treeDensity) {
                int height = heights[x][z];

                if (height <= waterLevel) continue;

                bool canPlaceTree = true;
                for (int dx = -1; dx <= 1 && canPlaceTree; ++dx) {
                    for (int dz = -1; dz <= 1 && canPlaceTree; ++dz) {
                        if (abs(heights[x + dx][z + dz] - height) > 2) {
                            canPlaceTree = false;
                        }
                    }
                }

                if (canPlaceTree) {
                    glm::vec3 treePos(x * voxelScale, (height + 1) * voxelScale, z * voxelScale);
                    auto tree = std::make_unique<Tree>(treePos, voxelScale);
                    auto treeVoxels = tree->getVoxels();
                    voxels.insert(voxels.end(), treeVoxels.begin(), treeVoxels.end());
                }
            }
        }
    }
}

std::vector<Voxel> BasicBiome::generateTerrain() {
    const int vox_width = config.gridConfig.vox_width;
    const int vox_depth = config.gridConfig.vox_depth;
    
    std::vector<Voxel> voxels;
    auto terrainHeights = generateHeightMap();

    // Generate terrain voxels
    for (int x = 0; x < vox_width; ++x) {
        for (int z = 0; z < vox_depth; ++z) {
            int height = terrainHeights[x][z];
            bool isNearWater = false;

            // Check 4-neighbor tiles for water proximity
            const int dx[] = {-1, 1, 0, 0};
            const int dz[] = {0, 0, -1, 1};
            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int nz = z + dz[i];
                if (nx >= 0 && nx < vox_width && nz >= 0 && nz < vox_depth) {
                    if (terrainHeights[nx][nz] < waterLevel) {
                        isNearWater = true;
                        break;
                    }
                }
            }

            // Fill terrain
            for (int y = 0; y <= height; ++y) {
                glm::vec3 position(x * voxelScale, y * voxelScale, z * voxelScale);
                int blockId;

                if (y == height) {
                    if (height <= waterLevel && isNearWater) {
                        blockId = 8; // Sand near water
                    } else {
                        blockId = 1; // Grass
                    }
                } else {
                    blockId = 2; // Dirt
                }

                voxels.emplace_back(position, blockId);
            }

            // Fill water
            for (int y = height + 1; y <= waterLevel; ++y) {
                glm::vec3 position(x * voxelScale, y * voxelScale, z * voxelScale);
                voxels.emplace_back(position, 7); // Water
            }
        }
    }

    // Add trees
    placeTrees(voxels, terrainHeights);

    return voxels;
}