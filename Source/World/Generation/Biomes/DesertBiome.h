#pragma once

#include "Biome.h"
#include <random>

class DesertBiome : public Biome {
public:
    DesertBiome(const Config& config)
        : Biome(config)
    {
        // Lower frequency for smoother, longer dunes
        frequency = 0.008f;
        // Lower amplitude for flatter terrain
        amplitude = static_cast<float>(config.gridConfig.vox_maxHeight) * 0.6f;
        // Lower water level for oases
        waterLevel = static_cast<int>(amplitude * 0.2f);
    }

    std::vector<Voxel> generateTerrain() override {
        return std::vector<Voxel>(); // Not used directly, handled by ChunkManager
    }
};