#ifndef VOXEL_H
#define VOXEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <map>

#include "../Utils/ConfigReader.h"

// Declare camera position as extern
extern glm::vec3 cameraPos;

struct BlockTexture {
    glm::vec2 top;
    glm::vec2 bottom;
    glm::vec2 front;
    glm::vec2 back;
    glm::vec2 left;
    glm::vec2 right;
};

struct Voxel {
    glm::vec3 position;
    unsigned int blockId; // Index in the texture atlas

    Voxel(const glm::vec3& pos = glm::vec3(0.0f), unsigned int id = 0) : position(pos), blockId(id) {}
};

class VoxelRenderer {
public:
    VoxelRenderer(Config& config);
    ~VoxelRenderer();

    void init();
    void render(const std::vector<Voxel>& voxels, const glm::mat4& view, const glm::mat4& projection);
    void setTextureAtlas(unsigned int textureId);
    void setBlockTexture(unsigned int blockId, const BlockTexture& textures);
    BlockTexture getBlockTexture(unsigned int blockId) const;

    // Add lighting setters
    void setLightDir(const glm::vec3& dir) { lightDir = dir; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientStrength(float strength) { ambientStrength = strength; }
    Config& localconfig;

private:
    unsigned int shaderProgram;
    unsigned int shadowMapShader;  // New shader for shadow mapping pass
    unsigned int instanceVBO;
    unsigned int VAO;
    unsigned int textureAtlasId;
    std::map<unsigned int, BlockTexture> blockTextures;
    
    // Shadow mapping
    unsigned int depthMapFBO;
    unsigned int depthMap;
    const unsigned int SHADOW_WIDTH = 4096*2;
    const unsigned int SHADOW_HEIGHT = 4096*2;
    
    // Lighting properties
    glm::vec3 lightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.4f;
    
    // Helper functions
    void initShadowMap();
    void renderShadowMap(const std::vector<Voxel>& voxels);
    unsigned int createShader(const char* vertexPath, const char* fragmentPath);

    std::vector<float> unitCubeVerticesWithAtlasUV;
};

#endif // VOXEL_H
