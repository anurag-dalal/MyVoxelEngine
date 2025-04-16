#pragma once

#include <string>

namespace ShaderUtils {
    unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
}