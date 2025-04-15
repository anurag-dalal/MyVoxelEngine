#ifndef VOXEL_H
#define VOXEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Declare camera position as extern
extern glm::vec3 cameraPos;

struct Voxel {
    glm::vec3 position;
    unsigned int blockId; // Index in the texture atlas

    Voxel(const glm::vec3& pos = glm::vec3(0.0f), unsigned int id = 0) : position(pos), blockId(id) {}
};

class VoxelRenderer {
public:
    VoxelRenderer();
    ~VoxelRenderer();

    void init();
    void render(const std::vector<Voxel>& voxels, const glm::mat4& view, const glm::mat4& projection);
    void setTextureAtlas(unsigned int textureId);

    // Add lighting setters
    void setLightDir(const glm::vec3& dir) { lightDir = dir; }
    void setLightColor(const glm::vec3& color) { lightColor = color; }
    void setAmbientStrength(float strength) { ambientStrength = strength; }

private:
    unsigned int shaderProgram;
    unsigned int instanceVBO;
    unsigned int VAO;
    unsigned int textureAtlasId;
    
    // Lighting properties
    glm::vec3 lightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.4f;
    
    // Shader loading utility
    unsigned int createShader(const char* vertexPath, const char* fragmentPath);
};

#endif // VOXEL_H
