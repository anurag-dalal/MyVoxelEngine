#pragma once

#include "Biome.h"
#include <random>

class AlpineBiome : public Biome {
public:
    AlpineBiome(const Config& config)
        : Biome(config)
    {
        // Higher frequency for detailed alpine terrain
        frequency = 0.014f;
        // Higher amplitude for alpine peaks
        amplitude = static_cast<float>(config.gridConfig.vox_maxHeight) * 1.0f;
        // Higher water level for alpine lakes
        waterLevel = static_cast<int>(amplitude * 0.5f);
    }

    std::vector<Voxel> generateTerrain() override {
        return std::vector<Voxel>(); // Not used directly, handled by ChunkManager
    }
};