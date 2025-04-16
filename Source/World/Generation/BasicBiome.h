#pragma once

#include "Biome.h"
#include <random>
#include "../../Models/Tree.h"
#include <memory>

class BasicBiome : public Biome {
private:
    std::vector<std::vector<int>> generateHeightMap();
    void placeTrees(std::vector<Voxel>& voxels, const std::vector<std::vector<int>>& heights);

public:
    BasicBiome(const Config& config);
    std::vector<Voxel> generateTerrain() override;
};