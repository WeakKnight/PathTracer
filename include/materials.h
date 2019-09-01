
//-------------------------------------------------------------------------------
///
/// \file       materials.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    2.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _MATERIALS_H_INCLUDED_
#define _MATERIALS_H_INCLUDED_
 
#include "scene.h"
 
//-------------------------------------------------------------------------------
 
class MtlBlinn : public Material
{
public:
    MtlBlinn() : diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f), glossiness(20.0f) {}
    virtual Color Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const;
 
    void SetDiffuse(Color dif) { diffuse = dif; }
    void SetSpecular(Color spec) { specular = spec; }
    void SetGlossiness(float gloss) { glossiness = gloss; }
 
    virtual void SetViewportMaterial(int subMtlID=0) const; // used for OpenGL display
 
private:
    Color diffuse, specular;
    float glossiness;
};

class MtlPhong : public Material
{
public:
    MtlPhong() : diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f), glossiness(20.0f) {}
    virtual Color Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const;
    
    void SetDiffuse(Color dif) { diffuse = dif; }
    void SetSpecular(Color spec) { specular = spec; }
    void SetGlossiness(float gloss) { glossiness = gloss; }
    
    virtual void SetViewportMaterial(int subMtlID=0) const; // used for OpenGL display
    
private:
    Color diffuse, specular;
    float glossiness;
};
 
//-------------------------------------------------------------------------------
 
#endif
