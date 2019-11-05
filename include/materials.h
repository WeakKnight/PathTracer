
//-------------------------------------------------------------------------------
///
/// \file       materials.h 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    10.0
/// \date       August 21, 2019
///
/// \brief Example source for CS 6620 - University of Utah.
///
//-------------------------------------------------------------------------------
 
#ifndef _MATERIALS_H_INCLUDED_
#define _MATERIALS_H_INCLUDED_
 
#include "scene.h"
#include <random>
#include "sampler.h"
 
//-------------------------------------------------------------------------------
 
class MtlBlinn : public Material
{
public:
    MtlBlinn() : diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f), glossiness(20.0f), 
                 reflection(0,0,0), refraction(0,0,0), absorption(0,0,0), ior(1),
                 reflectionGlossiness(0), refractionGlossiness(0) {}
    virtual Color Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount) const;
 
    void SetDiffuse     (Color dif)     { diffuse.SetColor(dif); }
    void SetSpecular    (Color spec)    { specular.SetColor(spec); }
    void SetGlossiness  (float gloss)   
	{ 
		glossiness = gloss; 
	}
 
    void SetReflection  (Color reflect) { reflection.SetColor(reflect); }
    void SetRefraction  (Color refract) { refraction.SetColor(refract); }
    void SetAbsorption  (Color absorp ) { absorption = absorp; }
    void SetRefractionIndex(float _ior) { ior = _ior; }
 
    void SetDiffuseTexture   (TextureMap *map)  { diffuse.SetTexture(map); }
    void SetSpecularTexture  (TextureMap *map)  { specular.SetTexture(map); }
    void SetReflectionTexture(TextureMap *map)  { reflection.SetTexture(map); }
    void SetRefractionTexture(TextureMap *map)  { refraction.SetTexture(map); }
	
	void SetNormalTexture(TextureMap* map) { normal = map; }
	void SetAOTexture(TextureMap* map) { ao = map; }

    void SetReflectionGlossiness(float gloss)   
	{ 
		reflectionGlossiness = gloss; 
		reflectNormalDist = std::normal_distribution<float>(0.0f, 0.39f * (gloss + 0.0000001f));
	}

    void SetRefractionGlossiness(float gloss)   
	{ 
		refractionGlossiness = gloss; 
		refractNormalDist = std::normal_distribution<float>(0.0f, 3.0f * (gloss + 0.0000001f));
	}
 
	void SetReflectNormalDistribution(float value)
	{
		if (value > 0.0f)
		{
			reflectNormalDistribution = true;
		}
		else
		{
			reflectNormalDistribution = false;
		}
	}

	void SetRefractNormalDistribution(float value)
	{
		if (value > 0.0f)
		{
			refractNormalDistribution = true;
		}
		else
		{
			refractNormalDistribution = false;
		}
	}

	void SetReflectAngelDistributioon(float angle)
	{
		reflectDistributionAngel = angle;
		hasReflectAngelDistribution = true;
		reflectAngelDist = std::normal_distribution<float>(reflectDistributionAngel, 0.39f * (reflectionGlossiness + 0.0000001f));
	}

    virtual void SetViewportMaterial(int subMtlID=0) const; // used for OpenGL display
 
private:
	Vec3f GenerateNormalWithGlossiness(const Vec3f& originalNormal, int type) const;

    TexturedColor diffuse, specular, reflection, refraction;
	TextureMap* normal = nullptr;
	TextureMap* ao = nullptr;

    float glossiness;
    Color absorption;
    float ior;  // index of refraction
    float reflectionGlossiness, refractionGlossiness;
	std::default_random_engine eng; 
	std::normal_distribution<float> reflectNormalDist; 
	std::normal_distribution<float> refractNormalDist;
	bool reflectNormalDistribution = false;
	bool refractNormalDistribution = false;

	bool hasReflectAngelDistribution = false;
	float reflectDistributionAngel = 0.0f;
	std::normal_distribution<float> reflectAngelDist;

	static QuasyMonteCarloCircleSampler* normalSampler;
};
 
//-------------------------------------------------------------------------------
 
class MultiMtl : public Material
{
public:
    virtual ~MultiMtl() { for ( unsigned int i=0; i<mtls.size(); i++ ) delete mtls[i]; }
 
    virtual Color Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount) const { 
        return hInfoContext.mainHitInfo.mtlID<(int)mtls.size() ? mtls[hInfoContext.mainHitInfo.mtlID]->Shade(rayContext,hInfoContext,lights,bounceCount) : Color(1,1,1); 
        }
 
    virtual void SetViewportMaterial(int subMtlID=0) const { if ( subMtlID<(int)mtls.size() ) mtls[subMtlID]->SetViewportMaterial(); }
 
    void AppendMaterial(Material *m) { mtls.push_back(m); }
 
private:
    std::vector<Material*> mtls;
};
 
//-------------------------------------------------------------------------------
 
#endif