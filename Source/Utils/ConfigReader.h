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
};

struct Config {
    WindowConfig window;
    TextureAtlasConfig textureAtlas;
    CameraConfig camera;
    PerformanceConfig performance;
    float voxelScale;
};

// Declaration only
Config loadConfig(const std::string& filename);
