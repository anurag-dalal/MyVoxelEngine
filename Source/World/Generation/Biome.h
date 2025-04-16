#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "../Voxel.h"
#include "../../Utils/ConfigReader.h"

class Biome {
protected:
    const Config& config;
    unsigned int textureAtlas;
    glm::vec3 lightDir;
    glm::vec3 lightColor;
    float ambientStrength;
    
    // Terrain generation parameters
    float frequency;
    float amplitude;
    int waterLevel;
    float voxelScale;

public:
    Biome(const Config& config);
    virtual ~Biome() = default;

    // Setters for lighting and texture
    void setTextureAtlas(unsigned int texture) { textureAtlas = texture; }
    void setLightDir(const glm::vec3& dir) { lightDir = dir; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientStrength(float strength) { ambientStrength = strength; }

    // Main function to generate terrain
    virtual std::vector<Voxel> generateTerrain() = 0;
    
    // Utility functions that might be useful for derived classes
    int getWaterLevel() const { return waterLevel; }
    float getVoxelScale() const { return voxelScale; }
};