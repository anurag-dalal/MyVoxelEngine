cmake_minimum_required(VERSION 3.10)
project(MyVoxelEngine)
# Add GLM
set(GLM_INCLUDE_DIR "Include/glm")
include_directories(${GLM_INCLUDE_DIR})
set(CMAKE_CXX_STANDARD 17)

# Add GLAD
add_library(glad Include/glad/glad.c)
target_include_directories(glad PUBLIC include)

# Find packages
find_package(OpenGL REQUIRED)


# Add GLM (optional but recommended for matrices)
# You can also add GLM as a submodule or download it to external/glm
# Uncomment below if you have GLM:
# add_subdirectory(external/glm)

# Include directories
include_directories(
    Include
    Shaders/vertex.glsl
    Shaders/fragment.glsl
)

# Source files
add_executable(${PROJECT_NAME} Source/Application.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/Shaders")
# Link libraries
target_link_libraries(${PROJECT_NAME}
    glad
    glfw
    glad
    ${OPENGL_gl_LIBRARY}
)
