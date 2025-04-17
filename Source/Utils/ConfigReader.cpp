#include "ConfigReader.h"
#include <nlohmann/json.hpp>
#include <fstream>

Config loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    nlohmann::json j;
    file >> j;

    Config config;
    config.window.width = j["window"]["width"];
    config.window.height = j["window"]["height"];
    config.window.title = j["window"]["title"];

    config.textureAtlas.path = j["textureAtlas"]["path"];
    config.textureAtlas.width = j["textureAtlas"]["width"];
    config.textureAtlas.height = j["textureAtlas"]["height"];
    config.textureAtlas.blocksPerRow = j["textureAtlas"]["blocksPerRow"];
    config.textureAtlas.blocksPerCol = j["textureAtlas"]["blocksPerCol"];

    config.camera.position = glm::vec3(j["camera"]["position"][0], j["camera"]["position"][1], j["camera"]["position"][2]);
    config.camera.front = glm::vec3(j["camera"]["front"][0], j["camera"]["front"][1], j["camera"]["front"][2]);
    config.camera.up = glm::vec3(j["camera"]["up"][0], j["camera"]["up"][1], j["camera"]["up"][2]);
    config.camera.yaw = j["camera"]["yaw"];
    config.camera.pitch = j["camera"]["pitch"];
    config.camera.fov = j["camera"]["fov"];

    config.gridConfig.vox_width = j["gridConfig"]["vox_width"];
    config.gridConfig.vox_depth = j["gridConfig"]["vox_depth"];
    config.gridConfig.vox_maxHeight = j["gridConfig"]["vox_maxHeight"];
    
    config.fullscreen.enabled = j["fullscreen"]["enabled"].get<bool>();
    config.fullscreen.borderless = j["fullscreen"]["borderless"].get<bool>();
    
    config.performance.numSamples = j["performance"]["numSamples"];
    config.performance.vsync = j["performance"]["vsync"];
    config.performance.targetFPS = j["performance"]["targetFPS"];
    
    // Parse world configuration if it exists
    if (j.contains("world")) {
        config.world.defaultBiome = j["world"]["defaultBiome"];
        config.world.biomeBlendFactor = j["world"]["biomeBlendFactor"];
        config.world.forceBiome = j["world"]["forceBiome"];
    } else {
        // Default values if not in config file
        config.world.defaultBiome = "PLAINS";
        config.world.biomeBlendFactor = 0.5f;
        config.world.forceBiome = false;
    }
    
    config.voxelScale = j["voxelScale"];
    config.skyname = j["skyname"];

    return config;
}
