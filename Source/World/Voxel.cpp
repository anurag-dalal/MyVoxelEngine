#include "Voxel.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

glm::vec2 getUV(float x, float y) {
    return glm::vec2(x / 16.0f, y / 16.0f);
}

// Updated cube vertices with UVs from the atlas
float halfSize = 0.1f/2; // Half size of the cube
// Texture coordinate origin (tile position in the atlas)
glm::vec2 texCoordBack  = {3, 0};
glm::vec2 texCoordFront = {3, 0};
glm::vec2 texCoordLeft  = {3, 0};
glm::vec2 texCoordRight = {3, 0};
glm::vec2 texCoordBottom= {2, 0};
glm::vec2 texCoordTop   = {0, 0};
float unitCubeVerticesWithAtlasUV[] = {
    // Back face (-Z)
    -halfSize, -halfSize, -halfSize, getUV(texCoordBack.x, texCoordBack.y + 1).x, getUV(texCoordBack.x, texCoordBack.y + 1).y, 0.0f, 0.0f, -1.0f,
     halfSize, -halfSize, -halfSize, getUV(texCoordBack.x + 1, texCoordBack.y + 1).x, getUV(texCoordBack.x + 1, texCoordBack.y + 1).y, 0.0f, 0.0f, -1.0f,
     halfSize,  halfSize, -halfSize, getUV(texCoordBack.x + 1, texCoordBack.y).x, getUV(texCoordBack.x + 1, texCoordBack.y).y, 0.0f, 0.0f, -1.0f,
     halfSize,  halfSize, -halfSize, getUV(texCoordBack.x + 1, texCoordBack.y).x, getUV(texCoordBack.x + 1, texCoordBack.y).y, 0.0f, 0.0f, -1.0f,
    -halfSize,  halfSize, -halfSize, getUV(texCoordBack.x, texCoordBack.y).x, getUV(texCoordBack.x, texCoordBack.y).y, 0.0f, 0.0f, -1.0f,
    -halfSize, -halfSize, -halfSize, getUV(texCoordBack.x, texCoordBack.y + 1).x, getUV(texCoordBack.x, texCoordBack.y + 1).y, 0.0f, 0.0f, -1.0f,

    // Front face (+Z)
    -halfSize, -halfSize,  halfSize, getUV(texCoordFront.x, texCoordFront.y + 1).x, getUV(texCoordFront.x, texCoordFront.y + 1).y, 0.0f, 0.0f, 1.0f,
     halfSize, -halfSize,  halfSize, getUV(texCoordFront.x + 1, texCoordFront.y + 1).x, getUV(texCoordFront.x + 1, texCoordFront.y + 1).y, 0.0f, 0.0f, 1.0f,
     halfSize,  halfSize,  halfSize, getUV(texCoordFront.x + 1, texCoordFront.y).x, getUV(texCoordFront.x + 1, texCoordFront.y).y, 0.0f, 0.0f, 1.0f,
     halfSize,  halfSize,  halfSize, getUV(texCoordFront.x + 1, texCoordFront.y).x, getUV(texCoordFront.x + 1, texCoordFront.y).y, 0.0f, 0.0f, 1.0f,
    -halfSize,  halfSize,  halfSize, getUV(texCoordFront.x, texCoordFront.y).x, getUV(texCoordFront.x, texCoordFront.y).y, 0.0f, 0.0f, 1.0f,
    -halfSize, -halfSize,  halfSize, getUV(texCoordFront.x, texCoordFront.y + 1).x, getUV(texCoordFront.x, texCoordFront.y + 1).y, 0.0f, 0.0f, 1.0f,

    // Left face (-X)
    -halfSize,  halfSize,  halfSize, getUV(texCoordLeft.x + 1, texCoordLeft.y).x, getUV(texCoordLeft.x + 1, texCoordLeft.y).y, -1.0f, 0.0f, 0.0f,
    -halfSize,  halfSize, -halfSize, getUV(texCoordLeft.x, texCoordLeft.y).x, getUV(texCoordLeft.x, texCoordLeft.y).y, -1.0f, 0.0f, 0.0f,
    -halfSize, -halfSize, -halfSize, getUV(texCoordLeft.x, texCoordLeft.y + 1).x, getUV(texCoordLeft.x, texCoordLeft.y + 1).y, -1.0f, 0.0f, 0.0f,
    -halfSize, -halfSize, -halfSize, getUV(texCoordLeft.x, texCoordLeft.y + 1).x, getUV(texCoordLeft.x, texCoordLeft.y + 1).y, -1.0f, 0.0f, 0.0f,
    -halfSize, -halfSize,  halfSize, getUV(texCoordLeft.x + 1, texCoordLeft.y + 1).x, getUV(texCoordLeft.x + 1, texCoordLeft.y + 1).y, -1.0f, 0.0f, 0.0f,
    -halfSize,  halfSize,  halfSize, getUV(texCoordLeft.x + 1, texCoordLeft.y).x, getUV(texCoordLeft.x + 1, texCoordLeft.y).y, -1.0f, 0.0f, 0.0f,

    // Right face (+X)
    halfSize,  halfSize,  halfSize, getUV(texCoordRight.x, texCoordRight.y).x, getUV(texCoordRight.x, texCoordRight.y).y, 1.0f, 0.0f, 0.0f,
    halfSize,  halfSize, -halfSize, getUV(texCoordRight.x + 1, texCoordRight.y).x, getUV(texCoordRight.x + 1, texCoordRight.y).y, 1.0f, 0.0f, 0.0f,
    halfSize, -halfSize, -halfSize, getUV(texCoordRight.x + 1, texCoordRight.y + 1).x, getUV(texCoordRight.x + 1, texCoordRight.y + 1).y, 1.0f, 0.0f, 0.0f,
    halfSize, -halfSize, -halfSize, getUV(texCoordRight.x + 1, texCoordRight.y + 1).x, getUV(texCoordRight.x + 1, texCoordRight.y + 1).y, 1.0f, 0.0f, 0.0f,
    halfSize, -halfSize,  halfSize, getUV(texCoordRight.x, texCoordRight.y + 1).x, getUV(texCoordRight.x, texCoordRight.y + 1).y, 1.0f, 0.0f, 0.0f,
    halfSize,  halfSize,  halfSize, getUV(texCoordRight.x, texCoordRight.y).x, getUV(texCoordRight.x, texCoordRight.y).y, 1.0f, 0.0f, 0.0f,

    // Bottom face (-Y)
    -halfSize, -halfSize, -halfSize, getUV(texCoordBottom.x, texCoordBottom.y).x, getUV(texCoordBottom.x, texCoordBottom.y).y, 0.0f, -1.0f, 0.0f,
     halfSize, -halfSize, -halfSize, getUV(texCoordBottom.x + 1, texCoordBottom.y).x, getUV(texCoordBottom.x + 1, texCoordBottom.y).y, 0.0f, -1.0f, 0.0f,
     halfSize, -halfSize,  halfSize, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).x, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).y, 0.0f, -1.0f, 0.0f,
     halfSize, -halfSize,  halfSize, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).x, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).y, 0.0f, -1.0f, 0.0f,
    -halfSize, -halfSize,  halfSize, getUV(texCoordBottom.x, texCoordBottom.y + 1).x, getUV(texCoordBottom.x, texCoordBottom.y + 1).y, 0.0f, -1.0f, 0.0f,
    -halfSize, -halfSize, -halfSize, getUV(texCoordBottom.x, texCoordBottom.y).x, getUV(texCoordBottom.x, texCoordBottom.y).y, 0.0f, -1.0f, 0.0f,

    // Top face (+Y)
    -halfSize,  halfSize, -halfSize, getUV(texCoordTop.x, texCoordTop.y).x, getUV(texCoordTop.x, texCoordTop.y).y, 0.0f, 1.0f, 0.0f,
     halfSize,  halfSize, -halfSize, getUV(texCoordTop.x + 1, texCoordTop.y).x, getUV(texCoordTop.x + 1, texCoordTop.y).y, 0.0f, 1.0f, 0.0f,
     halfSize,  halfSize,  halfSize, getUV(texCoordTop.x + 1, texCoordTop.y + 1).x, getUV(texCoordTop.x + 1, texCoordTop.y + 1).y, 0.0f, 1.0f, 0.0f,
     halfSize,  halfSize,  halfSize, getUV(texCoordTop.x + 1, texCoordTop.y + 1).x, getUV(texCoordTop.x + 1, texCoordTop.y + 1).y, 0.0f, 1.0f, 0.0f,
    -halfSize,  halfSize,  halfSize, getUV(texCoordTop.x, texCoordTop.y + 1).x, getUV(texCoordTop.x, texCoordTop.y + 1).y, 0.0f, 1.0f, 0.0f,
    -halfSize,  halfSize, -halfSize, getUV(texCoordTop.x, texCoordTop.y).x, getUV(texCoordTop.x, texCoordTop.y).y, 0.0f, 1.0f, 0.0f
};

// Constructor and Destructor
VoxelRenderer::VoxelRenderer() : shaderProgram(0), instanceVBO(0), VAO(0), textureAtlasId(0) {}
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

    // Set lighting uniforms
    glUniform3f(glGetUniformLocation(shaderProgram, "lightDir"), -0.2f, -1.0f, -0.3f);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &cameraPos[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), 0.4f);

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
