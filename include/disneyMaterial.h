#pragma once

#include "materials.h"
#include <random>
#include "sampler.h"
#include "disneyBrdf.h"
#include "utils.h"
#include "disneyBrdf.h"
#include "node.h"

class MtlDisney : public Material
{
public:
	MtlDisney() :
		albedo(0.5f, 0.5f, 0.5f)
	{
	}

	void SetAlbedo(Color dif)
	{
		albedo.SetColor(dif);
	}

	void SetRoughness(Color _roughness)
	{
		roughness.SetColor(_roughness);
	}
	void SetMetalness(Color _metalness)
	{
		metalness.SetColor(_metalness);
	}

	void SetAlbedoTexture(TextureMap* map)
	{
		albedo.SetTexture(map);
	}
	void SetRoughnessTexture(TextureMap* _roughness)
	{
		roughness.SetTexture(_roughness);
	}
	void SetMetalnessTexture(TextureMap* _metalness)
	{
		metalness.SetTexture(_metalness);
	}
	void SetNormalTexture(TextureMap* map)
	{
		normal = map;
	}

	void SetSpecular(float _specular)
	{
		specular = _specular;
	}

	void SetSpecularTint(float _specularTint)
	{
		specularTint = _specularTint;
	}

	void SetSheen(float _sheen)
	{
		sheen = _sheen;
	}

	void SetSheenTint(float _sheenTint)
	{
		sheenTint = _sheenTint;
	}

	void SetClearcoat(float _clearcoat)
	{
		clearcoat = _clearcoat;
	}

	void SetClearcoatGloss(float _clearcoatGloss)
	{
		clearcoatGloss = _clearcoatGloss;
	}

	void SetSubsurface(float _subsurface)
	{
		subsurface = _subsurface;
	}

	float InternalComputePdf(DisneyShadingInfo& shading, Vec3f& wi, const Vec3f& wo, Vec3f& brdfN)
	{
		Vec3f H = (wi + wo).GetNormalized();
		float NDotH = brdfN.Dot(H);
		float NDotL = brdfN.Dot(wi);
		float HDotL = H.Dot(wi);

		return brdf.DisneyPdf(shading, NDotH, NDotL, HDotL);
	}

	virtual void Sample(const HitInfo& hInfo, Vec3f& wi, const Vec3f& wo, float& probability)
	{
		DisneyShadingInfo shading;

		shading.baseColor =  albedo.SampleSrgb(hInfo.uvw, hInfo.duvw).ToVec();
		shading.roughness = roughness.Sample(hInfo.uvw, hInfo.duvw).r;
		shading.metallic = metalness.Sample(hInfo.uvw, hInfo.duvw).r;
		shading.clearcoat = clearcoat;
		shading.clearcoatGloss = clearcoatGloss;
		shading.sheen = sheen;
		shading.sheenTint = sheenTint;
		shading.specular = specular;
		shading.specularTint = specularTint;
		shading.subsurface = subsurface;

		shading.Clamp();
		shading.InitCSW();
		
		Vec3f N = hInfo.N;
		N.Normalize();
		Vec3f brdfN = N;
		if (this->normal)
		{
			Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
			brdfN = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
			brdfN.Normalize();
		}

		wi = brdf.DisneySample(shading, wo, brdfN);

		probability = InternalComputePdf(shading, wi, wo, brdfN);
	}

	virtual float ComputePdf(const HitInfo& hInfo, Vec3f& wi, const Vec3f& wo)
	{
		DisneyShadingInfo shading;

		shading.baseColor = albedo.SampleSrgb(hInfo.uvw, hInfo.duvw).ToVec();
		shading.roughness = roughness.Sample(hInfo.uvw, hInfo.duvw).r;
		shading.metallic = metalness.Sample(hInfo.uvw, hInfo.duvw).r;
		shading.clearcoat = clearcoat;
		shading.clearcoatGloss = clearcoatGloss;
		shading.sheen = sheen;
		shading.sheenTint = sheenTint;
		shading.specular = specular;
		shading.specularTint = specularTint;
		shading.subsurface = subsurface;

		shading.Clamp();
		shading.InitCSW();

		Vec3f N = hInfo.N;
		N.Normalize();
		Vec3f brdfN = N;
		if (this->normal)
		{
			Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
			brdfN = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
			brdfN.Normalize();
		}

		return InternalComputePdf(shading, wi, wo, brdfN);
	}

	virtual Color EvalBrdf(const HitInfo& hInfo, const Vec3f& wi, const Vec3f& wo, Vec3f& shadingNormal)
	{
		DisneyShadingInfo shading;

		shading.baseColor = albedo.SampleSrgb(hInfo.uvw, hInfo.duvw).ToVec();
		shading.roughness = roughness.Sample(hInfo.uvw, hInfo.duvw).r;
		shading.metallic = metalness.Sample(hInfo.uvw, hInfo.duvw).r;
		shading.clearcoat = clearcoat;
		shading.clearcoatGloss = clearcoatGloss;
		shading.sheen = sheen;
		shading.sheenTint = sheenTint;
		shading.specular = specular;
		shading.specularTint = specularTint;
		shading.subsurface = subsurface;

		shading.Clamp();
		shading.InitCSW();

		Vec3f N = hInfo.N;
		N.Normalize();
		Vec3f brdfN = N;
		if (this->normal)
		{
			Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
			brdfN = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
			brdfN.Normalize();
		}

		Vec3f H = (wi + wo).GetNormalized();
		float NDotH = brdfN.Dot(H);
		float NDotL = brdfN.Dot(wi);
		float HDotL = H.Dot(wi);
		float NDotV = brdfN.Dot(wo);

		Color emission = Color::Black();
		auto light = hInfo.node->GetLight();
		if (light)
		{
			emission = light->Le();
		}

		shadingNormal = brdfN;

		return emission + brdf.DisneyEval(shading, NDotL, NDotV, NDotH, HDotL);
	}

private:

	BrdfDisney brdf;
	TexturedColor albedo, metalness, roughness;
	TextureMap* normal = nullptr;
	float specular = 0.0f;
	//float anisotropy = 0.0f;
	float specularTint = 0.0f;
	float sheenTint = 0.0f;
	float sheen = 0.0f;
	float clearcoatGloss = 0.0f;
	float clearcoat = 0.0f;
	float subsurface = 0.0f;
};