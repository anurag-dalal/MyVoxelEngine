cmake_minimum_required(VERSION 3.10)
project(MyVoxelEngine)

# Add GLM
set(GLM_INCLUDE_DIR "Include/glm")
include_directories(${GLM_INCLUDE_DIR})

# Add Imgui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/Include/imgui)
set(IMGUI_BACKENDS_DIR ${IMGUI_DIR}/backends)

set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKENDS_DIR}/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_BACKENDS_DIR})

# Set path to NVML include and lib
include_directories(/usr/local/cuda-12.6/targets/x86_64-linux/include)

find_library(NVML_LIB nvidia-ml
  PATHS /usr/local/cuda-12.6/targets/x86_64-linux/lib
)

# Add GLAD
add_library(glad Include/glad/glad.c)
target_include_directories(glad PUBLIC Include)

# Find packages
find_package(OpenGL REQUIRED)

# Add GpuUsage header file
set(GPU_USAGE_HEADER ${CMAKE_SOURCE_DIR}/Source/Utils/GpuUsage.hpp)

# Include directories
include_directories(
    Include
)

# Add the JSON library include path
include_directories(${CMAKE_SOURCE_DIR}/Include/json/include)

# Source files
add_executable(${PROJECT_NAME} 
    Source/Application2.cpp
    Source/Utils/SystemUsage.cpp
    Source/Utils/ConfigReader.h
    Source/Utils/ConfigReader.cpp
    Source/Utils/HeightMapGenerator.h
    Source/Utils/HeightMapGenerator.cpp
    Source/Utils/ShaderUtils.h
    Source/Utils/ShaderUtils.cpp
    Source/Models/Model.h
    Source/Models/Tree.h
    Source/Models/Tree.cpp
    Source/Sky/ogldev_cubemap_texture.h
    Source/Sky/cubemap_texture.cpp
    Source/World/Voxel.h
    Source/World/Voxel.cpp
    Source/World/Generation/Biome.h
    Source/World/Generation/Biome.cpp
    Source/World/Generation/BasicBiome.h
    Source/World/Generation/BasicBiome.cpp
    Source/World/Chunk.h
    Source/World/Chunk.cpp
    Source/World/ChunkManager.h
    Source/World/ChunkManager.cpp
    Source/Player/Player.h
    Source/Player/Player.cpp
    Source/Player/PlayerController.h
    Source/Player/PlayerController.cpp
    Source/Player/PlayerConfigReader.h
    Source/Player/PlayerConfigReader.cpp
)

# Define paths for resources
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    SHADER_DIR="${CMAKE_SOURCE_DIR}/Shaders" 
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets" 
    CONFIG_FILE="${CMAKE_SOURCE_DIR}/Configs/config.json"
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    glad
    glfw
    imgui
    ${OPENGL_gl_LIBRARY}
    ${NVML_LIB}
)
