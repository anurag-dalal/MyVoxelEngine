#ifndef OGLDEV_CUBEMAP_TEXTURE_H
#define OGLDEV_CUBEMAP_TEXTURE_H

#include <string>
#include <glad/glad.h>

class CubemapTexture {
public:
    CubemapTexture(const std::string& right,
                   const std::string& left,
                   const std::string& top,
                   const std::string& bottom,
                   const std::string& front,
                   const std::string& back);
    ~CubemapTexture();
    bool Load();
    void Bind(GLenum TextureUnit);

private:
    std::string m_fileNames[6];
    GLuint m_textureObj;
};

#endif