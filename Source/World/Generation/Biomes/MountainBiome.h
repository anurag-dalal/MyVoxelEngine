#pragma once

#include "../Biome.h"
#include <random>

class MountainBiome : public Biome {
public:
    MountainBiome(const Config& config)
        : Biome(config)
    {
        // Higher frequency for more jagged terrain
        frequency = 0.015f;
        // Higher amplitude for tall mountains
        amplitude = static_cast<float>(config.gridConfig.vox_maxHeight) * 1.2f;
        // Moderate water level for mountain lakes
        waterLevel = static_cast<int>(amplitude * 0.3f);
    }

    std::vector<Voxel> generateTerrain() override {
        return std::vector<Voxel>(); // Not used directly, handled by ChunkManager
    }
};