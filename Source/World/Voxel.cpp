#include "Voxel.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>  // For glm::lookAt, glm::ortho
#include <glm/gtx/transform.hpp>         // Additional transformation functions
#include "../Utils/ShaderUtils.h"

glm::vec2 getUV(float x, float y) {
    return glm::vec2(x / 16.0f, y / 16.0f);
}



// Constructor and Destructor
VoxelRenderer::VoxelRenderer(Config& config) : shaderProgram(0), instanceVBO(0), VAO(0), textureAtlasId(0), depthMapFBO(0), depthMap(0), shadowMapShader(0), localconfig(config) {
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

    // Set the configuration
    localconfig = config;
    float halfSize = localconfig.voxelScale / 2;
    unitCubeVerticesWithAtlasUV = {
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
}
VoxelRenderer::~VoxelRenderer() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
    glDeleteProgram(shadowMapShader);
}

void VoxelRenderer::initShadowMap() {
    // Create framebuffer
    glGenFramebuffers(1, &depthMapFBO);
    
    // Create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer is not complete!" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Initialization
void VoxelRenderer::init() {
    // Create main shader program
    shaderProgram = ShaderUtils::createShaderProgram(
        std::string(SHADER_DIR) + "/voxel_vertex.glsl",
        std::string(SHADER_DIR) + "/voxel_fragment.glsl"
    );
    
    // Create shadow mapping shader program
    shadowMapShader = ShaderUtils::createShaderProgram(
        std::string(SHADER_DIR) + "/shadow_mapping.vert",
        std::string(SHADER_DIR) + "/shadow_mapping.frag"
    );

    // Initialize shadow mapping
    initShadowMap();
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex data buffer
    unsigned int cubeVBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // 8*6*6*4 = 1152 bytes
    // 8 floats per vertex (3 for position, 2 for UV, 3 for normal)
    // 6 faces, 6 vertices per face, 4 bytes per float
    glBufferData(GL_ARRAY_BUFFER, 1152, &unitCubeVerticesWithAtlasUV[0], GL_STATIC_DRAW);

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

void VoxelRenderer::renderShadowMap(const std::vector<Voxel>& voxels) {
    // Configure viewport to shadow map dimensions
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Create light space matrix
    float near_plane = 1.0f, far_plane = 100.0f;
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(-lightDir * 30.0f,  // Light position
                                     glm::vec3(0.0f),      // Look at origin
                                     glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    
    // Use shadow mapping shader
    glUseProgram(shadowMapShader);
    glUniformMatrix4fv(glGetUniformLocation(shadowMapShader, "lightSpaceMatrix"), 
                       1, GL_FALSE, &lightSpaceMatrix[0][0]);
    
    // Bind VAO and update instance buffer
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, voxels.size() * sizeof(Voxel), voxels.data(), GL_STATIC_DRAW);
    
    // Draw shadow map
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, voxels.size());
    
    // Reset framebuffer and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Rendering the voxels
void VoxelRenderer::render(const std::vector<Voxel>& voxels, const glm::mat4& view, const glm::mat4& projection) {
    if (shaderProgram == 0 || textureAtlasId == 0) {
        std::cerr << "Error: VoxelRenderer not properly initialized or texture not set.\n";
        return;
    }

    // First render pass: generate shadow map
    renderShadowMap(voxels);
    
    // Second render pass: render scene with shadows
    glViewport(0, 0, localconfig.window.width, localconfig.window.height); // TODO: Get actual window dimensions
    
    glUseProgram(shaderProgram);

    // Calculate light space matrix (same as in renderShadowMap)
    float near_plane = 1.0f, far_plane = 100.0f;
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(-lightDir * 30.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    
    // Create model matrix - translate to origin for now
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

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

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);

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
