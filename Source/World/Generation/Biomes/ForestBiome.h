#pragma once

#include "../Biome.h"
#include <random>
#include "../../../Models/Tree.h"
#include <memory>

class ForestBiome : public Biome {
public:
    ForestBiome(const Config& config)
        : Biome(config)
    {
        // Higher frequency for more varied terrain
        frequency = 0.01f;
        // More moderate amplitude for gentle hills
        amplitude = static_cast<float>(config.gridConfig.vox_maxHeight) * 0.85f;
        // Moderate water level
        waterLevel = static_cast<int>(amplitude * 0.4f);
    }

    std::vector<Voxel> generateTerrain() override {
        return std::vector<Voxel>(); // Not used directly, handled by ChunkManager
    }
};