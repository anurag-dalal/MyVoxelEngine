#pragma once

#include <string>
#include <glm/glm.hpp>

struct CameraConfig {
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    float yaw;
    float pitch;
    float fov;
};

struct TextureAtlasConfig {
    std::string path;
    int width;
    int height;
    int blocksPerRow;
    int blocksPerCol;
};

struct WindowConfig {
    int width;
    int height;
    std::string title;
};

struct PerformanceConfig {
    int numSamples;
    bool vsync;
    int targetFPS;
};

struct GridConfig {
    int vox_width;
    int vox_depth;
    int vox_maxHeight;
};

struct FullscreenConfig {
    bool enabled;
    bool borderless;
};

struct WorldConfig {
    std::string defaultBiome;
    float biomeBlendFactor;
    bool forceBiome;
};

struct Config {
    WindowConfig window;
    TextureAtlasConfig textureAtlas;
    CameraConfig camera;
    PerformanceConfig performance;
    GridConfig gridConfig;
    float voxelScale;
    FullscreenConfig fullscreen;
    std::string skyname;
    WorldConfig world;
};

// Declaration only
Config loadConfig(const std::string& filename);
