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
#include "World/ChunkManager.h" // Include the ChunkManager header

Config config = loadConfig(CONFIG_FILE);

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

// Camera
glm::vec3 cameraPos = config.camera.position;
glm::vec3 cameraFront = config.camera.front;
glm::vec3 cameraUp = config.camera.up;
float yaw = config.camera.yaw;
float pitch = config.camera.pitch;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Global chunk manager
ChunkManager* chunkManager = nullptr;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(lastY - ypos); // reversed
    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
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

    // Create and setup the biome
    BasicBiome biome(config);
    biome.setTextureAtlas(texture);
    biome.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    biome.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    biome.setAmbientStrength(0.4f);

    // Initialize the ChunkManager
    chunkManager = new ChunkManager(config);
    chunkManager->init(biome);

    glEnable(GL_DEPTH_TEST);
    const int NUM_SAMPLES = config.performance.numSamples;;
    std::vector<float> frameTimes(NUM_SAMPLES, 0.0f);
    int frameIndex = 0;
    SystemUsage usage;
    SystemUsageAsync usageAsync;
    glm::mat4 projection = glm::perspective(glm::radians(config.camera.fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

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

        processInput(window);

        // Blue background
        glClearColor(0.2f, 0.3f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update chunk loading based on camera position
        chunkManager->updateChunks(cameraPos);
        
        // Get all visible voxels from loaded chunks
        std::vector<Voxel> voxelsToRender = chunkManager->getVisibleVoxels();
        
        // Update stats
        chunksLoaded = 0;
        totalVoxels = voxelsToRender.size();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(config.camera.fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

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
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up chunk manager
    if (chunkManager) {
        delete chunkManager;
    }

    glfwTerminate();

    // Cleanup
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    delete skyboxTexture;

    return 0;
}