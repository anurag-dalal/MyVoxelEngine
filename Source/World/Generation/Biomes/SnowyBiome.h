#pragma once

#include "Biome.h"
#include <random>

class SnowyBiome : public Biome {
public:
    SnowyBiome(const Config& config)
        : Biome(config)
    {
        // Medium frequency for varied terrain
        frequency = 0.012f;
        // Moderate amplitude for gentle, snowy hills
        amplitude = static_cast<float>(config.gridConfig.vox_maxHeight) * 0.75f;
        // High water level for frozen lakes
        waterLevel = static_cast<int>(amplitude * 0.45f);
    }

    std::vector<Voxel> generateTerrain() override {
        return std::vector<Voxel>(); // Not used directly, handled by ChunkManager
    }
};