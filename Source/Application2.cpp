#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include "Utils/SystemUsage.cpp"
#include "World/Voxel.h" // Include the Voxel header
#include "Utils/ConfigReader.h"// Include the ConfigReader header
#include "Utils/HeightMapGenerator.h"
#include "Utils/ShaderUtils.h"
#include "Models/Tree.h"
#include "Sky/ogldev_cubemap_texture.h"
#include "World/Generation/BasicBiome.h"
#include "World/Generation/Biomes/ForestBiome.h"
#include "World/Generation/Biomes/DesertBiome.h"
#include "World/Generation/Biomes/MountainBiome.h"
#include "World/Generation/Biomes/SnowyBiome.h"
#include "World/Generation/Biomes/AlpineBiome.h"
#include "World/ChunkManager.h" // Include the ChunkManager header
#include "Player/Player.h" // Include the Player header
#include "Player/PlayerController.h" // Include the PlayerController header
#include "Player/PlayerConfigReader.h" // Include the PlayerConfigReader header

// Define path for player config file
#define PLAYER_CONFIG_FILE std::string(CONFIG_FILE).substr(0, std::string(CONFIG_FILE).find_last_of('/') + 1) + "player_config.json"

Config config = loadConfig(CONFIG_FILE);
PlayerConfig playerConfig = loadPlayerConfig(PLAYER_CONFIG_FILE);

// Biome control variables
bool isBiomeForced = false;
BiomeType forcedBiomeType = BiomeType::PLAINS;
float biomeBlendFactor = 0.5f;
bool showBiomeWindow = false;

// Helper function to convert BiomeType to string for display
std::string getBiomeTypeName(BiomeType type) {
    switch (type) {
        case BiomeType::PLAINS:      return "Plains";
        case BiomeType::FOREST:      return "Forest";
        case BiomeType::DESERT:      return "Desert";
        case BiomeType::MOUNTAINS:   return "Mountains";
        case BiomeType::SNOWY_PLAINS: return "Snowy Plains";
        case BiomeType::LAKE:        return "Lake";
        case BiomeType::ALPINE:      return "Alpine";
        default:                     return "Unknown";
    }
}

// Helper function to convert string to BiomeType
BiomeType getBiomeTypeFromString(const std::string& biomeStr) {
    if (biomeStr == "PLAINS") return BiomeType::PLAINS;
    if (biomeStr == "FOREST") return BiomeType::FOREST;
    if (biomeStr == "DESERT") return BiomeType::DESERT;
    if (biomeStr == "MOUNTAINS") return BiomeType::MOUNTAINS;
    if (biomeStr == "SNOWY_PLAINS") return BiomeType::SNOWY_PLAINS;
    if (biomeStr == "LAKE") return BiomeType::LAKE;
    if (biomeStr == "ALPINE") return BiomeType::ALPINE;
    
    // Return PLAINS as default if string doesn't match
    std::cerr << "Warning: Unknown biome type '" << biomeStr << "', defaulting to PLAINS" << std::endl;
    return BiomeType::PLAINS;
}

// Window size
const unsigned int SCR_WIDTH = config.window.width;
const unsigned int SCR_HEIGHT = config.window.height;

// Texture atlas dimensions (defined here as well for convenience)
const int ATLAS_WIDTH = config.textureAtlas.width;
const int ATLAS_HEIGHT = config.textureAtlas.height;
const int BLOCKS_PER_ROW = config.textureAtlas.blocksPerRow;
const int BLOCKS_PER_COL = config.textureAtlas.blocksPerCol;

// Voxel scale
const float VOXEL_SCALE = config.voxelScale;

// Player and controller
Player* player = nullptr;
PlayerController* playerController = nullptr;

// Mouse handling (will be managed by PlayerController)
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Global chunk manager
ChunkManager* chunkManager = nullptr;

// For camera position used by VoxelRenderer
glm::vec3 cameraPos = glm::vec3(0.0f);

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Forward input handling to the player controller
void processInput(GLFWwindow *window)
{
    if (playerController) {
        playerController->processKeyboardInput(deltaTime);
    }

    // Keep escape key handling in main application
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Toggle biome control window with B key
    static bool bKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!bKeyPressed) {
            showBiomeWindow = !showBiomeWindow;
            bKeyPressed = true;
            
            // Toggle mouse cursor for UI control
            if (showBiomeWindow) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
    } else {
        bKeyPressed = false;
    }
    
    // Toggle forced biome with F key
    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            isBiomeForced = !isBiomeForced;
            if (isBiomeForced) {
                // Apply the forced biome to the chunk manager
                chunkManager->setForcedBiome(forcedBiomeType);
                std::cout << "Forced biome mode enabled: " << getBiomeTypeName(forcedBiomeType) << std::endl;
            } else {
                // Clear forced biome
                chunkManager->clearForcedBiome();
                std::cout << "Forced biome mode disabled, natural biome generation restored" << std::endl;
            }
            fKeyPressed = true;
        }
    } else {
        fKeyPressed = false;
    }
    
    // Adjust biome blend factor with [ and ] keys
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) {
        biomeBlendFactor = std::max(0.0f, biomeBlendFactor - 0.01f);
        chunkManager->setBiomeBlendFactor(biomeBlendFactor);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) {
        biomeBlendFactor = std::min(1.0f, biomeBlendFactor + 0.01f);
        chunkManager->setBiomeBlendFactor(biomeBlendFactor);
    }
}

// Forward mouse movement to the player controller
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (playerController) {
        playerController->processMouseMovement(xpos, ypos);
    }
}

// Add skybox variables
unsigned int skyboxVAO, skyboxVBO;
unsigned int skyboxShader;
CubemapTexture* skyboxTexture;

float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// Add these function declarations at the top, outside main()
std::string loadShaderSource(const std::string& filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << "\n";
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    return ShaderUtils::createShaderProgram(vertexPath, fragmentPath);
}

int main()
{
    std::cout << "Loading configuration from: " << CONFIG_FILE << std::endl;
    std::cout << "Loading player configuration from: " << PLAYER_CONFIG_FILE << std::endl;
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Set up window
    GLFWwindow *window;
    if (config.fullscreen.enabled)
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        // Set window hints for fullscreen
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        
        if (config.fullscreen.borderless)
        {
            // Borderless fullscreen (windowed fullscreen)
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            window = glfwCreateWindow(mode->width, mode->height, config.window.title.c_str(), nullptr, nullptr);
            glfwSetWindowPos(window, 0, 0);
        }
        else
        {
            // True fullscreen
            window = glfwCreateWindow(mode->width, mode->height, config.window.title.c_str(), monitor, nullptr);
        }
    }
    else
    {
        window = glfwCreateWindow(config.window.width, config.window.height, config.window.title.c_str(), nullptr, nullptr);
    }

    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Configure VSync based on settings
    glfwSwapInterval(config.performance.vsync ? 1 : 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Load texture atlas
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Set texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int tx_width, tx_height, nrChannels;

    unsigned char *data = stbi_load((std::string(ASSETS_DIR) + "/atlas.png").c_str(), &tx_width, &tx_height, &nrChannels, 0);
    if (data)
    {
        if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tx_width, tx_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if (nrChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tx_width, tx_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        else
            std::cerr << "Error: Unsupported number of channels in texture\n";
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cerr << "Failed to load texture\n";
    }
    stbi_image_free(data);

    // Initialize the VoxelRenderer
    VoxelRenderer voxelRenderer(config);
    voxelRenderer.init();
    voxelRenderer.setTextureAtlas(texture);

    // Set lighting properties
    voxelRenderer.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    voxelRenderer.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    voxelRenderer.setAmbientStrength(0.4f);

    // Create and setup the biomes
    BasicBiome plainsBiome(config);
    plainsBiome.setTextureAtlas(texture);
    plainsBiome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    plainsBiome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    plainsBiome.setAmbientStrength(0.4f);

    ForestBiome forestBiome(config);
    forestBiome.setTextureAtlas(texture);
    forestBiome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    forestBiome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    forestBiome.setAmbientStrength(0.4f);

    DesertBiome desertBiome(config);
    desertBiome.setTextureAtlas(texture);
    desertBiome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    desertBiome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    desertBiome.setAmbientStrength(0.4f);

    MountainBiome mountainBiome(config);
    mountainBiome.setTextureAtlas(texture);
    mountainBiome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    mountainBiome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    mountainBiome.setAmbientStrength(0.4f);

    SnowyBiome snowyBiome(config);
    snowyBiome.setTextureAtlas(texture);
    snowyBiome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    snowyBiome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    snowyBiome.setAmbientStrength(0.4f);

    AlpineBiome alpineBiome(config);
    alpineBiome.setTextureAtlas(texture);
    alpineBiome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    alpineBiome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    alpineBiome.setAmbientStrength(0.4f);

    // Initialize the ChunkManager
    chunkManager = new ChunkManager(config);
    chunkManager->init(plainsBiome);
    
    // Register all biomes with the ChunkManager
    chunkManager->addBiome(BiomeType::FOREST, &forestBiome);
    chunkManager->addBiome(BiomeType::DESERT, &desertBiome);
    chunkManager->addBiome(BiomeType::MOUNTAINS, &mountainBiome);
    chunkManager->addBiome(BiomeType::SNOWY_PLAINS, &snowyBiome);
    chunkManager->addBiome(BiomeType::ALPINE, &alpineBiome);
    
    // Apply world biome settings from config.json
    if (config.world.forceBiome) {
        // Convert string biome name to BiomeType enum
        BiomeType configBiomeType = getBiomeTypeFromString(config.world.defaultBiome);
        
        // Set global biome variables to match config
        forcedBiomeType = configBiomeType;
        isBiomeForced = true;
        biomeBlendFactor = config.world.biomeBlendFactor;
        
        // Apply settings to chunk manager
        chunkManager->setForcedBiome(forcedBiomeType);
        chunkManager->setBiomeBlendFactor(biomeBlendFactor);
        
        std::cout << "World set to forced biome: " << getBiomeTypeName(forcedBiomeType) 
                  << " (Blend Factor: " << biomeBlendFactor << ")" << std::endl;
    }
    
    // Initialize the Player with the ChunkManager
    player = new Player(chunkManager, playerConfig.physics.height, playerConfig.physics.width);
    
    // Initialize the PlayerController
    playerController = new PlayerController(player, window);
    playerController->init(playerConfig);
    
    // Start player at a random spawn position
    player->spawnRandomly();

    glEnable(GL_DEPTH_TEST);
    const int NUM_SAMPLES = config.performance.numSamples;;
    std::vector<float> frameTimes(NUM_SAMPLES, 0.0f);
    int frameIndex = 0;
    SystemUsage usage;
    SystemUsageAsync usageAsync;

    // Initialize skybox
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Create and compile skybox shader
    skyboxShader = createShaderProgram("Shaders/skybox.vert", "Shaders/skybox.frag");
    std::string skyboxDir = std::string(ASSETS_DIR) + "/Clouds/" + config.skyname;

    skyboxTexture = new CubemapTexture(
        skyboxDir + "/px.png", // +X
        skyboxDir + "/nx.png", // -X
        skyboxDir + "/py.png", // +Y
        skyboxDir + "/ny.png", // -Y
        skyboxDir + "/pz.png", // +Z
        skyboxDir + "/nz.png"  // -Z
    );
    if (!skyboxTexture->Load()) {
        std::cerr << "Failed to load skybox textures!" << std::endl;
        return -1;
    }

    // Stats for chunk system
    int chunksLoaded = 0;
    int totalVoxels = 0;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frameTimes[frameIndex] = deltaTime * 1000.0f; // store in milliseconds
        frameIndex = (frameIndex + 1) % NUM_SAMPLES;

        // Frame timing control if vsync is disabled
        if (!config.performance.vsync && config.performance.targetFPS > 0)
        {
            float targetFrameTime = 1.0f / config.performance.targetFPS;
            float frameTime = (float)glfwGetTime() - currentFrame;
            if (frameTime < targetFrameTime)
            {
                float sleepTime = (targetFrameTime - frameTime) * 1000.0f - 0.1f;
                if (sleepTime > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds((int)sleepTime));
                while ((float)glfwGetTime() - currentFrame < targetFrameTime)
                    ; // Spin-wait for the remainder
            }
        }

        // Process user input
        processInput(window);
        
        // Get player position for camera view and chunk loading
        glm::vec3 playerPos = player->getPosition();
        cameraPos = playerPos + glm::vec3(0.0f, player->getCameraHeightOffset(), 0.0f);
        
        // Pass camera position to voxel renderer for shadow calculations
        voxelRenderer.setCameraPosition(cameraPos);

        // Blue background
        glClearColor(0.2f, 0.3f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update chunk loading based on player position
        chunkManager->updateChunks(playerPos);
        
        // Get all visible voxels from loaded chunks
        std::vector<Voxel> voxelsToRender = chunkManager->getVisibleVoxels();
        
        // Update stats
        chunksLoaded = 0;
        totalVoxels = voxelsToRender.size();

        // Get view matrix from player
        glm::mat4 view = player->getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(config.camera.fov), 
                                               (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // Draw skybox first
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShader);
        // Remove translation from view matrix for skybox
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        
        glBindVertexArray(skyboxVAO);
        skyboxTexture->Bind(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // Render the voxels using the VoxelRenderer
        voxelRenderer.render(voxelsToRender, view, projection);

        // GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::GetIO().FontGlobalScale = 2.5f;
        ImGui::NewFrame();

        float fps = 1.0f / deltaTime;
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
        ImGui::Begin("Performance");
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);
        ImGui::PlotLines("Frame Time (ms)", frameTimes.data(), NUM_SAMPLES, frameIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
        ImGui::Text("GPU Usage: %.1d MB", usage.getGpuMemoryUsageMB());
        ImGui::Text("RAM Usage: %ld MB", usage.getRamUsageMB());
        ImGui::Text("CPU Usage: %.1f%%", usageAsync.getCpuUsagePercent());
        ImGui::Separator();
        ImGui::Text("Chunks: %d", chunksLoaded);
        ImGui::Text("Voxels: %d", totalVoxels);
        ImGui::Separator();
        
        // Player information
        ImGui::Text("Player Position: (%.1f, %.1f, %.1f)", 
                   playerPos.x, playerPos.y, playerPos.z);
        ImGui::Text("On Ground: %s", player->isOnGround() ? "Yes" : "No");
        
        // Display controls from config
        ImGui::Text("Controls:");
        ImGui::Text("Move: %c/%c/%c/%c", 
                   playerConfig.controls.keyboard.moveForward - GLFW_KEY_A + 'A',
                   playerConfig.controls.keyboard.moveLeft - GLFW_KEY_A + 'A',
                   playerConfig.controls.keyboard.moveBackward - GLFW_KEY_A + 'A',
                   playerConfig.controls.keyboard.moveRight - GLFW_KEY_A + 'A');
        ImGui::Text("Jump: Space, Sprint: LShift, Respawn: R");
        
        // Get current chunk coordinates
        int chunkX, chunkZ;
        chunkManager->worldToChunkCoords(playerPos, chunkX, chunkZ);
        
        // Display current chunk and biome information
        ImGui::Text("Current Chunk: (%d, %d)", chunkX, chunkZ);
        
        ImGui::End();
        
        // Biome control window
        if (showBiomeWindow) {
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(SCR_WIDTH - 420, 20), ImGuiCond_FirstUseEver);
            ImGui::Begin("Biome Control", &showBiomeWindow);
            
            // Display current biome information
            BiomeType currentBiome = chunkManager->getCurrentBiomeAt(playerPos);
            ImGui::Text("Current Biome: %s", getBiomeTypeName(currentBiome).c_str());
            
            // Toggle forced biome mode
            if (ImGui::Checkbox("Force Biome", &isBiomeForced)) {
                if (isBiomeForced) {
                    chunkManager->setForcedBiome(forcedBiomeType);
                } else {
                    chunkManager->clearForcedBiome();
                }
            }
            
            ImGui::Separator();
            ImGui::Text("Select Biome:");
            
            // Radio buttons for biome selection
            static int selectedBiome = static_cast<int>(forcedBiomeType);
            if (ImGui::RadioButton("Plains", &selectedBiome, static_cast<int>(BiomeType::PLAINS))) {
                forcedBiomeType = BiomeType::PLAINS;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            if (ImGui::RadioButton("Forest", &selectedBiome, static_cast<int>(BiomeType::FOREST))) {
                forcedBiomeType = BiomeType::FOREST;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            if (ImGui::RadioButton("Desert", &selectedBiome, static_cast<int>(BiomeType::DESERT))) {
                forcedBiomeType = BiomeType::DESERT;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            if (ImGui::RadioButton("Mountains", &selectedBiome, static_cast<int>(BiomeType::MOUNTAINS))) {
                forcedBiomeType = BiomeType::MOUNTAINS;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            if (ImGui::RadioButton("Snowy Plains", &selectedBiome, static_cast<int>(BiomeType::SNOWY_PLAINS))) {
                forcedBiomeType = BiomeType::SNOWY_PLAINS;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            if (ImGui::RadioButton("Alpine", &selectedBiome, static_cast<int>(BiomeType::ALPINE))) {
                forcedBiomeType = BiomeType::ALPINE;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            if (ImGui::RadioButton("Lake", &selectedBiome, static_cast<int>(BiomeType::LAKE))) {
                forcedBiomeType = BiomeType::LAKE;
                if (isBiomeForced) chunkManager->setForcedBiome(forcedBiomeType);
            }
            
            ImGui::Separator();
            
            // Biome blend factor slider
            if (ImGui::SliderFloat("Biome Blend Factor", &biomeBlendFactor, 0.0f, 1.0f)) {
                chunkManager->setBiomeBlendFactor(biomeBlendFactor);
            }
            
            ImGui::Separator();
            
            // Regenerate chunk button
            if (ImGui::Button("Regenerate Current Chunks")) {
                int chunkX, chunkZ;
                chunkManager->worldToChunkCoords(playerPos, chunkX, chunkZ);
                
                // Regenerate current chunk and neighbors
                for (int dx = -2; dx <= 2; dx++) {
                    for (int dz = -2; dz <= 2; dz++) {
                        chunkManager->regenerateChunk(chunkX + dx, chunkZ + dz);
                    }
                }
            }
            
            // Display controls help
            ImGui::Separator();
            ImGui::Text("Biome Controls:");
            ImGui::Text("F - Toggle forced biome");
            ImGui::Text("[ ] - Adjust blend factor");
            ImGui::Text("B - Toggle this window");
            
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up
    delete playerController;
    delete player;
    delete chunkManager;
    delete skyboxTexture;
    
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}