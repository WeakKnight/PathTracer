#pragma once

#include "materials.h"
#include <random>
#include "sampler.h"
#include "brdf_cook_torrance.h"
#include "utils.h"

class MtlStandard : public Material
{
public:
	MtlStandard() : albedo(0.5f, 0.5f, 0.5f), 
	emission(0, 0, 0)
	{
	}

	void SetAlbedo(Color dif) { albedo.SetColor(dif); }

	void SetRoughness(Color _roughness) { roughness.SetColor(_roughness); }
	void SetMetalness(Color _metalness) { metalness.SetColor(_metalness); }

	void SetEmission(Color e) { emission.SetColor(e); }

	void SetAlbedoTexture(TextureMap* map) { albedo.SetTexture(map); }
	void SetEmissionTexture(TextureMap* map) { emission.SetTexture(map); }

	void SetRoughnessTexture(TextureMap* _roughness) { roughness.SetTexture(_roughness); }
	void SetMetalnessTexture(TextureMap* _metalness) { metalness.SetTexture(_metalness); }

	void SetNormalTexture(TextureMap* map) { normal = map; }
	void SetAOTexture(TextureMap* map) { ao = map; }

	virtual void Sample(const HitInfo& hInfo, Vec3f& wi, const Vec3f& wo, float& probability)
	{
		float roughnessValue = roughness.Sample(hInfo.uvw, hInfo.duvw).r;
		Vec3f N = hInfo.N;
		N.Normalize();
		Vec3f brdfN = N;
		if (this->normal)
		{
			Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
			brdfN = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
			brdfN.Normalize();
		}

		Vec3f b1, b2;
		BranchlessONB(brdfN, b1, b2);

		Vec3f sampleDir = ImportanceSampleGGX(roughnessValue, probability);
		sampleDir = brdfN * sampleDir.z + b1 * sampleDir.x + b2 * sampleDir.y;
		sampleDir.Normalize();

		wi = sampleDir;
	}

	virtual float ComputePdf(const HitInfo& hInfo, Vec3f& wi, const Vec3f& wo)
	{
		float roughnessValue = roughness.Sample(hInfo.uvw, hInfo.duvw).r;
		Vec3f N = hInfo.N;
		N.Normalize();
		Vec3f brdfN = N;
		if (this->normal)
		{
			Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
			brdfN = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
			brdfN.Normalize();
		}

		float cosTheta = wi.Dot(brdfN);
		float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
		float a = roughnessValue * roughnessValue;
		float bottom = (a * a - 1.0f) * cosTheta * cosTheta + 1.0f;
		bottom = bottom * bottom;

		return a * a * cosTheta * sinTheta * INVERSE_PI / bottom;;
	}

	virtual Color EvalBrdf(const HitInfo& hInfo, const Vec3f& wi, const Vec3f& wo, Vec3f& shadingNormal)
	{
		Color albedoColor = albedo.SampleSrgb(hInfo.uvw, hInfo.duvw);
		// albedoColor = Color(powf(albedoColor.r, 2.2f), powf(albedoColor.g, 2.2f), powf(albedoColor.b, 2.2f));

		float roughnessValue = roughness.Sample(hInfo.uvw, hInfo.duvw).r;
		if (roughnessValue <= 0.0f)
		{
			roughnessValue = 0.001f;
		}
		float metalnessValue = metalness.Sample(hInfo.uvw, hInfo.duvw).r;

		Vec3f N = hInfo.N;
		N.Normalize();
		Vec3f brdfN = N;
		if (this->normal)
		{
			Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
			brdfN = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
			brdfN.Normalize();
		}

		shadingNormal = brdfN;

		return brdf.BRDF(wi, wo, N, albedoColor, roughnessValue, metalnessValue);
	}

private:
	BrdfCookTorrance brdf;

	TexturedColor albedo, emission, metalness, roughness;
	TextureMap* normal = nullptr;
	TextureMap* ao = nullptr;
};

