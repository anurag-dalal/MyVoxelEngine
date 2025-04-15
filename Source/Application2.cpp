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
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#include "Utils/GpuUsage.cpp"
#include "World/Voxel.h" // Include the Voxel header
#include "Utils/ConfigReader.h"// Include the ConfigReader header
#include "Utils/HeightMapGenerator.h"

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

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Set up window
    GLFWwindow *window = glfwCreateWindow(config.window.width, config.window.height, config.window.title.c_str(), NULL, NULL);

    if (window == NULL)
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

    int width, height, nrChannels;

    unsigned char *data = stbi_load((std::string(ASSETS_DIR) + "/atlas.png").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        if (nrChannels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else if (nrChannels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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
    VoxelRenderer voxelRenderer;
    voxelRenderer.init();
    voxelRenderer.setTextureAtlas(texture);

    // Set lighting properties
    voxelRenderer.setLightDir(glm::vec3(-0.2f, -1.0f, -0.3f));
    voxelRenderer.setLightColor(glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow light color
    // voxelRenderer.setLightColor(glm::vec3(1.0f, 1.0f, 1.0f));
    voxelRenderer.setAmbientStrength(0.4f);

    // Create a large number of voxels
    std::vector<Voxel> voxelsToRender;
    int n = 64; // Example: 32x32x32 grid of voxels
    // for (int x = 0; x < n; ++x) {
    //     for (int y = 0; y < n; ++y) {
    //         for (int z = 0; z < n; ++z) {
    //             //if (rand() % 100 > 10) // 10% chance
    //               //  continue;
    //             voxelsToRender.emplace_back(glm::vec3(x*config.voxelScale, y*config.voxelScale, z*config.voxelScale), (x + y * n + z * n * n) % 256); // Example block ID
    //         }
    //     }
    // }

    // HeightMapGenerator generator(n, n, 0.0f, static_cast<float>(n - 1));
    // auto heightmap = generator.generateRandom();

    // for (int x = 0; x < n; ++x) {
    //     for (int z = 0; z < n; ++z) {
    //         int height = static_cast<int>(heightmap[z][x]); // z is row, x is column

    //         for (int y = 0; y <= height; ++y) {
    //             glm::vec3 position(x * config.voxelScale, y * config.voxelScale, z * config.voxelScale);
    //             int blockId = y == height ? 1 : 2; // Example: top block is grass, below is dirt
    //             voxelsToRender.emplace_back(position, blockId);
    //         }
    //     }
    // }
    // Grid size
    const int vox_width = 128;
    const int vox_depth = 128;
    const int vox_maxHeight = 64;
    float voxelScale = config.voxelScale; // From your config

    // Noise scaling factors
    float frequency = 0.005f; // Controls terrain frequency
    float amplitude = static_cast<float>(vox_maxHeight); // Controls max height

    for (int x = 0; x < vox_width; ++x) {
        for (int z = 0; z < vox_depth; ++z) {
            // Generate a height value using Perlin noise (normalized to [0, 1])
            float noiseValue = stb_perlin_noise3(x * frequency, 0.0f, z * frequency, 0, 0, 0);
            noiseValue = (noiseValue + 1.0f) / 2.0f; // Remap from [-1,1] to [0,1]
            int height = static_cast<int>(noiseValue * amplitude);

            // Fill voxels up to the height
            for (int y = 0; y <= height; ++y) {
                glm::vec3 position(x * voxelScale, y * voxelScale, z * voxelScale);

                // You can get fancier with block types here
                int blockId = (y == height) ? 1 : 2; // Top is grass, below is dirt
                voxelsToRender.emplace_back(position, blockId);
            }
        }
    }
    glEnable(GL_DEPTH_TEST);
    const int NUM_SAMPLES = config.performance.numSamples;;
    std::vector<float> frameTimes(NUM_SAMPLES, 0.0f);
    int frameIndex = 0;
    GpuUsage gpu;
    glm::mat4 projection = glm::perspective(glm::radians(config.camera.fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
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
        

        // Render the voxels using the VoxelRenderer
        voxelRenderer.render(voxelsToRender, view, projection);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::GetIO().FontGlobalScale = 2.5f;
        ImGui::NewFrame();

        float fps = 1.0f / deltaTime;
        ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Always);
        ImGui::Begin("Performance");
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);
        ImGui::PlotLines("Frame Time (ms)", frameTimes.data(), NUM_SAMPLES, frameIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
        ImGui::Text("GPU Usage: %.1d MB", gpu.get_usage());
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
    return 0;
}