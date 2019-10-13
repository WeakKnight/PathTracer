
//-------------------------------------------------------------------------------
///
/// \file       materials.h
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    7.0
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
    MtlBlinn() : diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f), glossiness(20.0f),
    reflection(0,0,0), refraction(0,0,0), absorption(0,0,0), ior(1) {}
    virtual Color Shade(RayContext const &rayContext, const HitInfo &hInfo, const LightList &lights, int bounceCount) const;
    
    void SetDiffuse     (Color dif)     { diffuse.SetColor(dif); }
    void SetSpecular    (Color spec)    { specular.SetColor(spec); }
    void SetGlossiness  (float gloss)   { glossiness = gloss; }
    
    void SetReflection  (Color reflect) { reflection.SetColor(reflect); }
    void SetRefraction  (Color refract) { refraction.SetColor(refract); }
    void SetAbsorption  (Color absorp ) { absorption = absorp; }
    void SetRefractionIndex(float _ior) { ior = _ior; }
    
    void SetDiffuseTexture   (TextureMap *map)  { diffuse.SetTexture(map); }
    void SetSpecularTexture  (TextureMap *map)  { specular.SetTexture(map); }
    void SetReflectionTexture(TextureMap *map)  { reflection.SetTexture(map); }
    void SetRefractionTexture(TextureMap *map)  { refraction.SetTexture(map); }
    
    virtual void SetViewportMaterial(int subMtlID=0) const; // used for OpenGL display
    
private:
    TexturedColor diffuse, specular, reflection, refraction;
    float glossiness;
    Color absorption;
    float ior;  // index of refraction
};

//-------------------------------------------------------------------------------

class MultiMtl : public Material
{
public:
    virtual ~MultiMtl() { for ( unsigned int i=0; i<mtls.size(); i++ ) delete mtls[i]; }
    
    virtual Color Shade(RayContext const &rayContext, const HitInfo &hInfo, const LightList &lights, int bounceCount) const { return hInfo.mtlID<(int)mtls.size() ? mtls[hInfo.mtlID]->Shade(rayContext,hInfo,lights,bounceCount) : Color(1,1,1); }
    
    virtual void SetViewportMaterial(int subMtlID=0) const { if ( subMtlID<(int)mtls.size() ) mtls[subMtlID]->SetViewportMaterial(); }
    
    void AppendMaterial(Material *m) { mtls.push_back(m); }
    
private:
    std::vector<Material*> mtls;
};

//-------------------------------------------------------------------------------

#endif
