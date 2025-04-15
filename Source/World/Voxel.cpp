#include "Voxel.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

glm::vec2 getUV(float x, float y) {
    return glm::vec2(x / 16.0f, y / 16.0f);
}

// Updated cube vertices with default UVs - these will be updated per block type
float halfSize = 0.1f/2;
float unitCubeVerticesWithAtlasUV[] = {
    // Back face (-Z)
    -halfSize, -halfSize, -halfSize, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
     halfSize, -halfSize, -halfSize, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
     halfSize,  halfSize, -halfSize, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
     halfSize,  halfSize, -halfSize, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    -halfSize,  halfSize, -halfSize, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
    -halfSize, -halfSize, -halfSize, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,

    // Front face (+Z)
    -halfSize, -halfSize,  halfSize, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     halfSize, -halfSize,  halfSize, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     halfSize,  halfSize,  halfSize, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
     halfSize,  halfSize,  halfSize, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -halfSize,  halfSize,  halfSize, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -halfSize, -halfSize,  halfSize, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,

    // Left face (-X)
    -halfSize,  halfSize,  halfSize, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -halfSize,  halfSize, -halfSize, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -halfSize, -halfSize, -halfSize, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -halfSize, -halfSize, -halfSize, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -halfSize, -halfSize,  halfSize, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
    -halfSize,  halfSize,  halfSize, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    // Right face (+X)
    halfSize,  halfSize,  halfSize, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    halfSize,  halfSize, -halfSize, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    halfSize, -halfSize, -halfSize, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    halfSize, -halfSize, -halfSize, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    halfSize, -halfSize,  halfSize, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    halfSize,  halfSize,  halfSize, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    // Bottom face (-Y)
    -halfSize, -halfSize, -halfSize, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
     halfSize, -halfSize, -halfSize, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
     halfSize, -halfSize,  halfSize, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
     halfSize, -halfSize,  halfSize, 1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -halfSize, -halfSize,  halfSize, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
    -halfSize, -halfSize, -halfSize, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,

    // Top face (+Y)
    -halfSize,  halfSize, -halfSize, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     halfSize,  halfSize, -halfSize, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
     halfSize,  halfSize,  halfSize, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
     halfSize,  halfSize,  halfSize, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -halfSize,  halfSize,  halfSize, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -halfSize,  halfSize, -halfSize, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f
};

// Constructor and Destructor
VoxelRenderer::VoxelRenderer() : shaderProgram(0), instanceVBO(0), VAO(0), textureAtlasId(0) {
    // Set up default block textures
    
    // Grass block (ID 1)
    BlockTexture grassBlock;
    grassBlock.top = glm::vec2(0, 0);      // Green grass top
    grassBlock.bottom = glm::vec2(2, 0);    // Dirt bottom
    grassBlock.front = grassBlock.back = grassBlock.left = grassBlock.right = glm::vec2(3, 0);  // Grass side
    blockTextures[1] = grassBlock;

    // Dirt block (ID 2)
    BlockTexture dirtBlock;
    dirtBlock.top = dirtBlock.bottom = dirtBlock.front = 
    dirtBlock.back = dirtBlock.left = dirtBlock.right = glm::vec2(2, 0);  // All dirt
    blockTextures[2] = dirtBlock;

    // Stone block (ID 3)
    BlockTexture stoneBlock;
    stoneBlock.top = stoneBlock.bottom = stoneBlock.front =
    stoneBlock.back = stoneBlock.left = stoneBlock.right = glm::vec2(1, 0);  // All stone
    blockTextures[3] = stoneBlock;

    // Wood block (ID 4)
    BlockTexture woodBlock;
    woodBlock.top = woodBlock.bottom = glm::vec2(4, 0);  // Wood end texture
    woodBlock.front = woodBlock.back = woodBlock.left = woodBlock.right = glm::vec2(5, 0);  // Wood side texture
    blockTextures[4] = woodBlock;

    // Leaves block (ID 5)
    BlockTexture leavesBlock;
    leavesBlock.top = leavesBlock.bottom = leavesBlock.front =
    leavesBlock.back = leavesBlock.left = leavesBlock.right = glm::vec2(6, 0);  // All leaves
    blockTextures[5] = leavesBlock;

    // Lava block (ID 6)
    BlockTexture lavaBlock;
    lavaBlock.top = lavaBlock.bottom = lavaBlock.front =
    lavaBlock.back = lavaBlock.left = lavaBlock.right = glm::vec2(7, 0);  // All lava
    blockTextures[6] = lavaBlock;

    // Water block (ID 7)
    BlockTexture waterBlock;
    waterBlock.top = waterBlock.bottom = waterBlock.front =
    waterBlock.back = waterBlock.left = waterBlock.right = glm::vec2(8, 0);  // All water
    blockTextures[7] = waterBlock;
}
VoxelRenderer::~VoxelRenderer() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteVertexArrays(1, &VAO);
}
unsigned int VoxelRenderer::createShader(const char* vertexPath, const char* fragmentPath) {
    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;
    unsigned int program = 0;
    int success;
    char infoLog[512];

    // Vertex Shader
    std::ifstream vShaderFile;
    std::stringstream vShaderStream;
    vShaderFile.open(vertexPath);
    if (vShaderFile.is_open()) {
        vShaderStream << vShaderFile.rdbuf();
        vShaderFile.close();
        std::string vertexCode = vShaderStream.str();
        const char* vShaderCode = vertexCode.c_str();

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    } else {
        std::cerr << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESFULLY_READ\n";
    }

    // Fragment Shader
    std::ifstream fShaderFile;
    std::stringstream fShaderStream;
    fShaderFile.open(fragmentPath);
    if (fShaderFile.is_open()) {
        fShaderStream << fShaderFile.rdbuf();
        fShaderFile.close();
        std::string fragmentCode = fShaderStream.str();
        const char* fShaderCode = fragmentCode.c_str();

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    } else {
        std::cerr << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESFULLY_READ\n";
    }

    // Shader Program
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}
// Initialization
void VoxelRenderer::init() {
    shaderProgram = createShader((std::string(SHADER_DIR) + "/voxel_vertex.glsl").c_str(),
                                 (std::string(SHADER_DIR) + "/voxel_fragment.glsl").c_str());

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex data buffer
    unsigned int cubeVBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unitCubeVerticesWithAtlasUV), unitCubeVerticesWithAtlasUV, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UV attribute
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Normal attribute
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // Instance data buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Instance position attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Voxel), (void*)offsetof(Voxel, position));
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    // Instance block ID attribute
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Voxel), (void*)offsetof(Voxel, blockId));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
    glDeleteBuffers(1, &cubeVBO);
}

// Rendering the voxels
void VoxelRenderer::render(const std::vector<Voxel>& voxels, const glm::mat4& view, const glm::mat4& projection) {
    if (shaderProgram == 0 || textureAtlasId == 0) {
        std::cerr << "Error: VoxelRenderer not properly initialized or texture not set.\n";
        return;
    }

    glUseProgram(shaderProgram);

    // Set matrix uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);

    // Set lighting uniforms using member variables
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, &lightDir[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, &lightColor[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &cameraPos[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);

    // Set texture uniforms
    glUniform1i(glGetUniformLocation(shaderProgram, "textureAtlas"), 0);
    glUniform1f(glGetUniformLocation(shaderProgram, "atlasSize"), 16.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlasId);

    glBindVertexArray(VAO);

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, voxels.size() * sizeof(Voxel), voxels.data(), GL_STATIC_DRAW);

    // Draw instanced cubes
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, voxels.size());

    glBindVertexArray(0);
}

// Set texture atlas
void VoxelRenderer::setTextureAtlas(unsigned int textureId) {
    textureAtlasId = textureId;
}

void VoxelRenderer::setBlockTexture(unsigned int blockId, const BlockTexture& textures) {
    blockTextures[blockId] = textures;
}

BlockTexture VoxelRenderer::getBlockTexture(unsigned int blockId) const {
    auto it = blockTextures.find(blockId);
    if (it != blockTextures.end()) {
        return it->second;
    }
    // Return default texture coordinates if block ID not found
    BlockTexture defaultTexture;
    defaultTexture.top = defaultTexture.bottom = defaultTexture.front = 
    defaultTexture.back = defaultTexture.left = defaultTexture.right = glm::vec2(0, 0);
    return defaultTexture;
}
