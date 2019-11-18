
//-------------------------------------------------------------------------------
///
/// \file       texture.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    1.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _TEXTURE_H_INCLUDED_
#define _TEXTURE_H_INCLUDED_
 
#include "scene.h"
#include "glm/vec4.hpp"
 
//-------------------------------------------------------------------------------
 
class TextureFile : public Texture
{
public:
    TextureFile() : width(0), height(0) {}
    bool Load();
    virtual Color Sample(Vec3f const &uvw) const;
private:

    std::vector<Color24> data8bit;
	std::vector<Color> data16bit;

	bool isHDR = false;
    int width, height;
};
 
//-------------------------------------------------------------------------------
 
class TextureChecker : public Texture
{
public:
    TextureChecker() : color1(0,0,0), color2(1,1,1) {}
    void SetColor1(const Color &c) { color1=c; }
    void SetColor2(const Color &c) { color2=c; }
    virtual Color Sample(Vec3f const &uvw) const;
private:
    Color color1, color2;
};
 
//-------------------------------------------------------------------------------
 
#endif
