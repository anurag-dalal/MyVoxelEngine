cmake_minimum_required(VERSION 3.10)
project(MyVoxelEngine)
# Add GLM
set(GLM_INCLUDE_DIR "Include/glm")
include_directories(${GLM_INCLUDE_DIR})
# Add Imgui
# Add ImGui source files
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
target_include_directories(glad PUBLIC include)

# Find packages
find_package(OpenGL REQUIRED)

# Add GpuUsage header file
set(GPU_USAGE_HEADER ${CMAKE_SOURCE_DIR}/Source/Utils/GpuUsage.hpp)
# Add GLM (optional but recommended for matrices)
# You can also add GLM as a submodule or download it to external/glm
# Uncomment below if you have GLM:
# add_subdirectory(external/glm)

# Include directories
include_directories(
    Include
    Shaders/vertex.glsl
    Shaders/fragment.glsl
    Shaders/voxel_fragment.glsl
    Shaders/voxel_fragment.glsl
)
# Add the submodule include path
include_directories(${CMAKE_SOURCE_DIR}/Include/json/include)
# Source files
add_executable(${PROJECT_NAME} 
                Source/Application2.cpp
                Source/Utils/SystemUsage.cpp
                Source/Utils/ConfigReader.h
                Source/Utils/ConfigReader.cpp
                Source/Utils/HeightMapGenerator.h
                Source/Utils/HeightMapGenerator.cpp
                Source/Models/Model.h
                Source/Models/Tree.h
                Source/Models/Tree.cpp
                Source/World/Voxel.h
                Source/World/Voxel.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/Shaders" ASSETS_DIR="${CMAKE_SOURCE_DIR}/Assets" CONFIG_FILE="${CMAKE_SOURCE_DIR}/Configs/config.json")
# Link libraries
target_link_libraries(${PROJECT_NAME}
    glad
    glfw
    glad
    imgui
    ${OPENGL_gl_LIBRARY}
    ${NVML_LIB}
)
