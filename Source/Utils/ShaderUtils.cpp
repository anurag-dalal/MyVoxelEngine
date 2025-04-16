#include "ShaderUtils.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace ShaderUtils {
    unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
        unsigned int vertexShader = 0;
        unsigned int fragmentShader = 0;
        unsigned int program = 0;
        int success;
        char infoLog[512];

        // Load and compile vertex shader
        std::ifstream vShaderFile(vertexPath);
        if (!vShaderFile.is_open()) {
            std::cerr << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESFULLY_READ: " << vertexPath << "\n";
            return 0;
        }
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        vShaderFile.close();
        std::string vertexCode = vShaderStream.str();
        const char* vShaderCode = vertexCode.c_str();

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            return 0;
        }

        // Load and compile fragment shader
        std::ifstream fShaderFile(fragmentPath);
        if (!fShaderFile.is_open()) {
            std::cerr << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESFULLY_READ: " << fragmentPath << "\n";
            glDeleteShader(vertexShader);
            return 0;
        }
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        fShaderFile.close();
        std::string fragmentCode = fShaderStream.str();
        const char* fShaderCode = fragmentCode.c_str();

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            glDeleteShader(vertexShader);
            return 0;
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
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }
}