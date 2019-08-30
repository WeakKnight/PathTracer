#pragma once

#include "glad/glad.h"

class Texture2D
{
    public:

    Texture2D();
    ~Texture2D();

    void SetData(unsigned char* data, GLuint InWidth, GLuint InHeight, GLenum colorFormat = GL_RGB);    
    void Bind() const;

    public:

    GLuint Id;
    GLuint Width;
    GLuint Height;
    unsigned char* Data;
};


