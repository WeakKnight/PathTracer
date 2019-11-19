
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
#include "brdf_cook_torrance.h"
//-------------------------------------------------------------------------------
 
class MtlBlinn : public Material
{
public:
    MtlBlinn() : diffuse(0.5f,0.5f,0.5f), specular(0.7f,0.7f,0.7f), glossiness(20.0f), 
                 reflection(0,0,0), refraction(0,0,0), emission(0, 0, 0), absorption(0,0,0), ior(1),
                 reflectionGlossiness(0), refractionGlossiness(0) {}
    virtual Color Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount, int indirectLightBounce) const;
 
	virtual Color IndirectLightShade(const Vec3f& N, RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount, int indirectLightBounce) const;

    void SetDiffuse     (Color dif)     { diffuse.SetColor(dif); }
    void SetSpecular    (Color spec)    { specular.SetColor(spec); }
	
	void SetRoughness(Color _roughness) { roughness.SetColor(_roughness); }
	void SetMetalness(Color _metalness) { metalness.SetColor(_metalness); }

    void SetGlossiness  (float gloss)   
	{ 
		glossiness = gloss; 
	}
	void SetEmission(Color e) { emission.SetColor(e); }
 
    void SetReflection  (Color reflect) { reflection.SetColor(reflect); }
    void SetRefraction  (Color refract) { refraction.SetColor(refract); }
    void SetAbsorption  (Color absorp ) { absorption = absorp; }
    void SetRefractionIndex(float _ior) { ior = _ior; }
 
    void SetDiffuseTexture   (TextureMap *map)  { diffuse.SetTexture(map); }
    void SetSpecularTexture  (TextureMap *map)  { specular.SetTexture(map); }
    void SetReflectionTexture(TextureMap *map)  { reflection.SetTexture(map); }
    void SetRefractionTexture(TextureMap *map)  { refraction.SetTexture(map); }
	void SetEmissionTexture(TextureMap* map) { emission.SetTexture(map); }

	void SetRoughnessTexture(TextureMap* _roughness) { roughness.SetTexture(_roughness); }
	void SetMetalnessTexture(TextureMap* _metalness) { metalness.SetTexture(_metalness); }

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

    virtual void SetViewportMaterial(int subMtlID=0) const; // used for OpenGL display
 
private:
	Color Lambert(const Vec3f& wi,const Vec3f& wo, const Vec3f& n, const Vec3f& x, const HitInfo& hitInfo) const;
	Color Specular(const Vec3f& wi, const Vec3f& wo, const Vec3f& n, const Vec3f& x, const HitInfo& hitInfo) const;

	Light* LiForDirect(const Vec3f& x, const Vec3f& n, const LightList& lights, float& inverseProbability) const;

private:
	BrdfCookTorrance brdf;
	Vec3f GenerateNormalWithGlossiness(const Vec3f& originalNormal, int type) const;

    TexturedColor diffuse, specular, reflection, refraction, emission, metalness, roughness;
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

	static QuasyMonteCarloCircleSampler* normalSampler;
};
 
//-------------------------------------------------------------------------------
 
class MultiMtl : public Material
{
public:
    virtual ~MultiMtl() { for ( unsigned int i=0; i<mtls.size(); i++ ) delete mtls[i]; }
 
    virtual Color Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount, int indirectLightBounce) const {
        return hInfoContext.mainHitInfo.mtlID<(int)mtls.size() ? mtls[hInfoContext.mainHitInfo.mtlID]->Shade(rayContext,hInfoContext,lights, bounceCount, indirectLightBounce) : Color(1,1,1);
        }
	virtual Color IndirectLightShade(const Vec3f& N, RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount, int indirectLightBounce) const
	{
		Color result;
		return result;
	}
 
    virtual void SetViewportMaterial(int subMtlID=0) const { if ( subMtlID<(int)mtls.size() ) mtls[subMtlID]->SetViewportMaterial(); }
 
    void AppendMaterial(Material *m) { mtls.push_back(m); }
 
private:
    std::vector<Material*> mtls;
};
 
//-------------------------------------------------------------------------------
 
#endif