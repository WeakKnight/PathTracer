#pragma once

#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include "scene.h"

#include <cmath>
#include "constants.h"

#include "spdlog/spdlog.h"

using namespace cy;

class BrdfCookTorrance
{
public:

	Color BRDF(const Vec3f& L, const Vec3f& V, const Vec3f& N, Color albedo, float roughness, float metalness) const
	{
		Vec3f H = (V + L).GetNormalized();

		Color F0 = Color(0.04f, 0.04f, 0.04f);
		F0 = Color(
			F0.r * (1.0f - metalness) + albedo.r * metalness,
			F0.g * (1.0f - metalness) + albedo.g * metalness,
			F0.b * (1.0f - metalness) + albedo.b * metalness);
		Color F = FresnelSchlick(F0, V, H);

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);

		Color numerator = NDF * G * F;

		float denominator = 4.0f * Max<float>(N.Dot(V), 0.0f) * Max<float>(N.Dot(L), 0.0f);

		Color specular = numerator / Max<float>(denominator, 0.001f);


		Color kS = F;
		Color kD = Color(1.0f, 1.0f, 1.0f) - kS;

		// when metalness = 1.0, no diffuse
		kD = kD * (1.0f - metalness);

		Color lambert = kD * albedo * INVERSE_PI;

		return (lambert + specular);
	}

private:

	float DistributionGGX(Vec3f N, Vec3f H, float roughness) const
	{
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = Max<float>(N.Dot(H), 0.0f);
		float NdotH2 = NdotH * NdotH;

		float nom = a2;
		float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
		denom = PI * denom * denom;

		return nom / denom;
	}

	float GeometrySchlickGGX(float NdotV, float roughness) const
	{
		float k = roughness * roughness * 0.5f;

		float nom = NdotV;
		float denom = NdotV * (1.0f - k) + k;

		return nom / denom;
	}

	float GeometrySmith(const Vec3f& N, const Vec3f& V, const Vec3f& L, float roughness) const
	{
		float NdotV = Max<float>(N.Dot(V), 0.0f);
		float NdotL = Max<float>(N.Dot(L), 0.0f);
		float ggx1 = GeometrySchlickGGX(NdotV, roughness);
		float ggx2 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}

	Color FresnelSchlick(Color F0, const Vec3f& V, const Vec3f H) const
	{
		float VDotH = Max<float>(V.Dot(H), 0.0f);
		return F0 + (1.0f - F0) * pow(1.0f - VDotH, 5.0);
	}
};