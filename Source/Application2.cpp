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
#include "Models/Tree.h"
#include "Sky/ogldev_cubemap_texture.h"

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
    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;
    unsigned int program = 0;
    int success;
    char infoLog[512];

    // Load and compile vertex shader
    std::string vertexCode = loadShaderSource(vertexPath);
    const char* vShaderCode = vertexCode.c_str();
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vShaderCode, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Load and compile fragment shader
    std::string fragmentCode = loadShaderSource(fragmentPath);
    const char* fShaderCode = fragmentCode.c_str();
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create and link shader program
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
    voxelRenderer.setLightColor(glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow light color
    // voxelRenderer.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    voxelRenderer.setAmbientStrength(0.4f);

    // Create a large number of voxels
    std::vector<Voxel> voxelsToRender;
    std::vector<std::unique_ptr<Model>> models;
    
    // Grid size
    const int vox_width = config.gridConfig.vox_width;
    const int vox_depth = config.gridConfig.vox_depth;
    const int vox_maxHeight = config.gridConfig.vox_maxHeight;
    float voxelScale = config.voxelScale;

    // Store terrain heights for tree placement
    std::vector<std::vector<int>> terrainHeights(vox_width, std::vector<int>(vox_depth));

    // Noise scaling factors
    float frequency = 0.005f;
    float amplitude = static_cast<float>(vox_maxHeight);

    // Generate terrain
    for (int x = 0; x < vox_width; ++x) {
        for (int z = 0; z < vox_depth; ++z) {
            float noiseValue = stb_perlin_noise3(x * frequency, 0.0f, z * frequency, 0, 0, 0);
            noiseValue = (noiseValue + 1.0f) / 2.0f;
            int height = static_cast<int>(noiseValue * amplitude);
            terrainHeights[x][z] = height;

            // Fill voxels up to the height
            for (int y = 0; y <= height; ++y) {
                glm::vec3 position(x * voxelScale, y * voxelScale, z * voxelScale);
                int blockId = (y == height) ? 1 : 2;
                voxelsToRender.emplace_back(position, blockId);
            }
        }
    }

    // Random number generation for tree placement
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> treeDist(0.0, 1.0);
    float treeDensity = 0.01f; // 1% chance of a tree at each valid position

    // Place trees
    for (int x = 2; x < vox_width - 2; ++x) {
        for (int z = 2; z < vox_depth - 2; ++z) {  // Fixed z loop condition
            // Only place trees on grass blocks with some probability
            if (treeDist(gen) < treeDensity) {
                int height = terrainHeights[x][z];
                
                // Check if surrounding terrain is relatively flat (no more than 2 blocks difference)
                bool canPlaceTree = true;
                for (int dx = -1; dx <= 1 && canPlaceTree; ++dx) {
                    for (int dz = -1; dz <= 1 && canPlaceTree; ++dz) {  // Fixed dz loop condition
                        if (abs(terrainHeights[x + dx][z + dz] - height) > 2) {
                            canPlaceTree = false;
                        }
                    }
                }

                if (canPlaceTree) {
                    // Apply voxel scale to the tree position
                    glm::vec3 treePos(x * voxelScale, (height + 1) * voxelScale, z * voxelScale);
                    models.push_back(std::make_unique<Tree>(treePos, voxelScale));
                }
            }
        }
    }

    // Add model voxels to render list
    for (const auto& model : models) {
        auto modelVoxels = model->getVoxels();
        voxelsToRender.insert(voxelsToRender.end(), modelVoxels.begin(), modelVoxels.end());
    }

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

    // Load skybox textures
    skyboxTexture = new CubemapTexture(
        "Assets/Clouds/clearsky/px.png", // +X
        "Assets/Clouds/clearsky/nx.png", // -X
        "Assets/Clouds/clearsky/py.png", // +Y
        "Assets/Clouds/clearsky/ny.png", // -Y
        "Assets/Clouds/clearsky/pz.png", // +Z
        "Assets/Clouds/clearsky/nz.png"  // -Z
    );
    if (!skyboxTexture->Load()) {
        std::cerr << "Failed to load skybox textures!" << std::endl;
        return -1;
    }

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        frameTimes[frameIndex] = deltaTime * 1000.0f; // store in milliseconds
        frameIndex = (frameIndex + 1) % NUM_SAMPLES;

        processInput(window);

        // Blue background
        glClearColor(0.2f, 0.3f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_Always);
        ImGui::Begin("Performance");
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);
        ImGui::PlotLines("Frame Time (ms)", frameTimes.data(), NUM_SAMPLES, frameIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
        ImGui::Text("GPU Usage: %.1d MB", usage.getGpuMemoryUsageMB());
        ImGui::Text("RAM Usage: %ld MB", usage.getRamUsageMB());
        ImGui::Text("CPU Usage: %.1f%%", usageAsync.getCpuUsagePercent());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    // Cleanup
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    delete skyboxTexture;

    return 0;
}