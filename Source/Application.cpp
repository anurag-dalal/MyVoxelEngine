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

#include "Utils/GpuUsage.cpp"

// Window size
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1600;

// Texture atlas dimensions
const int ATLAS_WIDTH = 256;
const int ATLAS_HEIGHT = 256;
const int BLOCKS_PER_ROW = 16;
const int BLOCKS_PER_COL = 16;

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
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

// Vertex Shader
std::string loadShaderSource(const std::string &filePath)
{
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open())
    {
        std::cerr << "Failed to open shader file: " << filePath << "\n";
        return "";
    }

    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    return shaderStream.str();
}

const std::string vertexShaderPath = std::string(SHADER_DIR) + "/vertex.glsl";
const std::string fragmentShaderPath = std::string(SHADER_DIR) + "/fragment.glsl";

std::string vertexShaderCode = loadShaderSource(vertexShaderPath);
std::string fragmentShaderCode = loadShaderSource(fragmentShaderPath);

const char *vertexShaderSource = vertexShaderCode.c_str();
const char *fragmentShaderSource = fragmentShaderCode.c_str();


glm::vec2 getTextureCoordinates(int blockId) {
    int row = blockId / BLOCKS_PER_ROW;
    int col = blockId % BLOCKS_PER_COL;

    float uMin = (float)col / BLOCKS_PER_ROW;
    float vMin = (float)row / BLOCKS_PER_COL;
    float uMax = (float)(col + 1) / BLOCKS_PER_ROW;
    float vMax = (float)(row + 1) / BLOCKS_PER_COL;

    // We only need the min and max for each face, the vertex data will interpolate.
    // For a single block, we can return the bottom-left corner as the base.
    return glm::vec2(uMin, vMin);
}

float halfSize = 0.1f;
const float atlasSize = 16.0f;

// Texture coordinate origin (tile position in the atlas)
glm::vec2 texCoordBack  = {3, 0};
glm::vec2 texCoordFront = {3, 0};
glm::vec2 texCoordLeft  = {3, 0};
glm::vec2 texCoordRight = {3, 0};
glm::vec2 texCoordBottom= {2, 0};
glm::vec2 texCoordTop   = {0, 0};

glm::vec2 getUV(float x, float y) {
    return glm::vec2(x / 16.0f, y / 16.0f);
}


float cubeVerticesWithAtlasUV[] = {
    // --- Back face ---
    -halfSize, -halfSize, -halfSize, getUV(texCoordBack.x    , texCoordBack.y + 1).x, getUV(texCoordBack.x    , texCoordBack.y + 1).y,
     halfSize, -halfSize, -halfSize, getUV(texCoordBack.x + 1, texCoordBack.y + 1).x, getUV(texCoordBack.x + 1, texCoordBack.y + 1).y,
     halfSize,  halfSize, -halfSize, getUV(texCoordBack.x + 1, texCoordBack.y    ).x, getUV(texCoordBack.x + 1, texCoordBack.y    ).y,
     halfSize,  halfSize, -halfSize, getUV(texCoordBack.x + 1, texCoordBack.y    ).x, getUV(texCoordBack.x + 1, texCoordBack.y    ).y,
    -halfSize,  halfSize, -halfSize, getUV(texCoordBack.x    , texCoordBack.y    ).x, getUV(texCoordBack.x    , texCoordBack.y    ).y,
    -halfSize, -halfSize, -halfSize, getUV(texCoordBack.x    , texCoordBack.y + 1).x, getUV(texCoordBack.x    , texCoordBack.y + 1).y,

    // --- Front face ---
    -halfSize, -halfSize,  halfSize, getUV(texCoordFront.x    , texCoordFront.y + 1).x, getUV(texCoordFront.x    , texCoordFront.y + 1).y,
     halfSize, -halfSize,  halfSize, getUV(texCoordFront.x + 1, texCoordFront.y + 1).x, getUV(texCoordFront.x + 1, texCoordFront.y + 1).y,
     halfSize,  halfSize,  halfSize, getUV(texCoordFront.x + 1, texCoordFront.y    ).x, getUV(texCoordFront.x + 1, texCoordFront.y    ).y,
     halfSize,  halfSize,  halfSize, getUV(texCoordFront.x + 1, texCoordFront.y    ).x, getUV(texCoordFront.x + 1, texCoordFront.y    ).y,
    -halfSize,  halfSize,  halfSize, getUV(texCoordFront.x    , texCoordFront.y    ).x, getUV(texCoordFront.x    , texCoordFront.y    ).y,
    -halfSize, -halfSize,  halfSize, getUV(texCoordFront.x    , texCoordFront.y + 1).x, getUV(texCoordFront.x    , texCoordFront.y + 1).y,

    // Left face (fixed UV rotation)
    - halfSize,  halfSize,  halfSize, getUV(texCoordLeft.x + 1, texCoordLeft.y    ).x, getUV(texCoordLeft.x + 1, texCoordLeft.y    ).y,
    - halfSize,  halfSize, -halfSize, getUV(texCoordLeft.x    , texCoordLeft.y    ).x, getUV(texCoordLeft.x    , texCoordLeft.y    ).y,
    - halfSize, -halfSize, -halfSize, getUV(texCoordLeft.x    , texCoordLeft.y + 1).x, getUV(texCoordLeft.x    , texCoordLeft.y + 1).y,
    - halfSize, -halfSize, -halfSize, getUV(texCoordLeft.x    , texCoordLeft.y + 1).x, getUV(texCoordLeft.x    , texCoordLeft.y + 1).y,
    - halfSize, -halfSize,  halfSize, getUV(texCoordLeft.x + 1, texCoordLeft.y + 1).x, getUV(texCoordLeft.x + 1, texCoordLeft.y + 1).y,
    - halfSize,  halfSize,  halfSize, getUV(texCoordLeft.x + 1, texCoordLeft.y    ).x, getUV(texCoordLeft.x + 1, texCoordLeft.y    ).y,


    // Right face (fixed UV rotation)
    halfSize,  halfSize,  halfSize, getUV(texCoordRight.x    , texCoordRight.y    ).x, getUV(texCoordRight.x    , texCoordRight.y    ).y,
    halfSize,  halfSize, -halfSize, getUV(texCoordRight.x + 1, texCoordRight.y    ).x, getUV(texCoordRight.x + 1, texCoordRight.y    ).y,
    halfSize, -halfSize, -halfSize, getUV(texCoordRight.x + 1, texCoordRight.y + 1).x, getUV(texCoordRight.x + 1, texCoordRight.y + 1).y,
    halfSize, -halfSize, -halfSize, getUV(texCoordRight.x + 1, texCoordRight.y + 1).x, getUV(texCoordRight.x + 1, texCoordRight.y + 1).y,
    halfSize, -halfSize,  halfSize, getUV(texCoordRight.x    , texCoordRight.y + 1).x, getUV(texCoordRight.x    , texCoordRight.y + 1).y,
    halfSize,  halfSize,  halfSize, getUV(texCoordRight.x    , texCoordRight.y    ).x, getUV(texCoordRight.x    , texCoordRight.y    ).y,


    // --- Bottom face ---
    -halfSize, -halfSize, -halfSize, getUV(texCoordBottom.x    , texCoordBottom.y    ).x, getUV(texCoordBottom.x    , texCoordBottom.y    ).y,
     halfSize, -halfSize, -halfSize, getUV(texCoordBottom.x + 1, texCoordBottom.y    ).x, getUV(texCoordBottom.x + 1, texCoordBottom.y    ).y,
     halfSize, -halfSize,  halfSize, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).x, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).y,
     halfSize, -halfSize,  halfSize, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).x, getUV(texCoordBottom.x + 1, texCoordBottom.y + 1).y,
    -halfSize, -halfSize,  halfSize, getUV(texCoordBottom.x    , texCoordBottom.y + 1).x, getUV(texCoordBottom.x    , texCoordBottom.y + 1).y,
    -halfSize, -halfSize, -halfSize, getUV(texCoordBottom.x    , texCoordBottom.y    ).x, getUV(texCoordBottom.x    , texCoordBottom.y    ).y,

    // --- Top face ---
    -halfSize,  halfSize, -halfSize, getUV(texCoordTop.x    , texCoordTop.y    ).x, getUV(texCoordTop.x    , texCoordTop.y    ).y,
     halfSize,  halfSize, -halfSize, getUV(texCoordTop.x + 1, texCoordTop.y    ).x, getUV(texCoordTop.x + 1, texCoordTop.y    ).y,
     halfSize,  halfSize,  halfSize, getUV(texCoordTop.x + 1, texCoordTop.y + 1).x, getUV(texCoordTop.x + 1, texCoordTop.y + 1).y,
     halfSize,  halfSize,  halfSize, getUV(texCoordTop.x + 1, texCoordTop.y + 1).x, getUV(texCoordTop.x + 1, texCoordTop.y + 1).y,
    -halfSize,  halfSize,  halfSize, getUV(texCoordTop.x    , texCoordTop.y + 1).x, getUV(texCoordTop.x    , texCoordTop.y + 1).y,
    -halfSize,  halfSize, -halfSize, getUV(texCoordTop.x    , texCoordTop.y    ).x, getUV(texCoordTop.x    , texCoordTop.y    ).y,
};


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MyVoxelEngine", NULL, NULL);
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

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

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

    // Update vertex data with texture coordinates based on the atlas
    std::vector<float> texturedCubeVertices;
    float blockWidthUV = 1.0f / BLOCKS_PER_ROW;
    float blockHeightUV = 1.0f / BLOCKS_PER_COL;

    auto push_face = [&](float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float u0, float v0, float u1, float v1, float u2, float v2, float u3, float v3) {
        texturedCubeVertices.insert(texturedCubeVertices.end(), {x0, y0, z0, u0, v1});
        texturedCubeVertices.insert(texturedCubeVertices.end(), {x1, y1, z1, u1, v1});
        texturedCubeVertices.insert(texturedCubeVertices.end(), {x2, y2, z2, u1, v0});

        texturedCubeVertices.insert(texturedCubeVertices.end(), {x2, y2, z2, u1, v0});
        texturedCubeVertices.insert(texturedCubeVertices.end(), {x3, y3, z3, u0, v0});
        texturedCubeVertices.insert(texturedCubeVertices.end(), {x0, y0, z0, u0, v1});
    };

    // Define texture coordinates for each face using block ID 0 for simplicity
    glm::vec2 baseTexCoord = getTextureCoordinates(0);
    float uMin = baseTexCoord.x;
    float vMin = baseTexCoord.y;
    float uMax = uMin + blockWidthUV;
    float vMax = vMin + blockHeightUV;

    // Back face
    push_face(-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f, uMin, vMin, uMax, vMin, uMax, vMax, uMin, vMax);
    // Front face
    push_face(-0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, uMin, vMin, uMax, vMin, uMax, vMax, uMin, vMax);
    // Left face
    push_face(-0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, uMin, vMin, uMax, vMin, uMax, vMax, uMin, vMax);
    // Right face
    push_face( 0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, uMin, vMin, uMax, vMin, uMax, vMax, uMin, vMax);
    // Bottom face
    push_face(-0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f, uMin, vMin, uMax, vMin, uMax, vMax, uMin, vMax);
    // Top face
    push_face(-0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f, uMin, vMin, uMax, vMin, uMax, vMax, uMin, vMax);


    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerticesWithAtlasUV), cubeVerticesWithAtlasUV, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);
    const int NUM_SAMPLES = 100;
    std::vector<float> frameTimes(NUM_SAMPLES, 0.0f);
    int frameIndex = 0;
    GpuUsage gpu;

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

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture0"), 0);

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

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
        ImGui::Text("GPU Usage: %.1f%%", gpu.get_usage());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}
