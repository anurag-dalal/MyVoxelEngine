#include "Biome.h"

Biome::Biome(const Config& config) 
    : config(config),
      textureAtlas(0),
      lightDir(0.0f),
      lightColor(1.0f),
      ambientStrength(0.4f),
      frequency(0.008f),
      amplitude(static_cast<float>(config.gridConfig.vox_maxHeight)),
      waterLevel(static_cast<int>(amplitude * 0.35f)),
      voxelScale(config.voxelScale)
{}