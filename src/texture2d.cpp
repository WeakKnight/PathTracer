#include "texture2d.h"

namespace RayTracing
{


    Texture2D::Texture2D()
    :
    Width(0),
    Height(0),
    Id(0)
    {
        glGenTextures(1, &Id);
    }

    Texture2D::~Texture2D()
    {
        glDeleteTextures(1, &Id);
        delete Data;
    }

    void Texture2D::SetData(unsigned char* data, GLuint InWidth, GLuint InHeight, GLenum colorFormat)
    {
        Width = InWidth;
        Height = InHeight;

        Data = data;

        glBindTexture(GL_TEXTURE_2D, Id);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, InWidth, InHeight, 0, colorFormat, GL_UNSIGNED_BYTE, data);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture2D::Bind() const
    {
        glBindTexture(GL_TEXTURE_2D, Id);
    }
}
