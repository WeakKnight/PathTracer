#pragma once

#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include "scene.h"

#include <cmath>
#include "constants.h"

using namespace cy;

class BrdfCookTorrance
{
public:

	Color BRDFIndirect(const Vec3f& L, const Vec3f& V, Vec3f& N, Color albedo, float roughness, float metalness) const
	{
		float cosTheta = Max<float>(N.Dot(L), 0.0f);

		Color F0 = Color(0.04f, 0.04f, 0.04f);
		F0 = Color(
			F0.r * (1.0f - metalness) + albedo.r * metalness,
			F0.g * (1.0f - metalness) + albedo.g * metalness,
			F0.b * (1.0f - metalness) + albedo.b * metalness);
		Color F = FresnelSchlickRoughness(cosTheta, F0, roughness);

		Vec3f H = (V + L).GetNormalized();
		float NDF = DistributionGGXDirect(N, H, roughness);
		float G = GeometrySmithDirect(N, V, L, roughness);

		Color numerator = NDF * G * F;
		float denominator = 4.0f * Max<float>(N.Dot(V), 0.0f) * Max<float>(N.Dot(L), 0.0f);

		Color specular = numerator / Max<float>(denominator, 0.001f);

		Color kS = F;
		Color kD = Color(1.0f, 1.0f, 1.0f) - kS;

		kD = kD * (1.0f - metalness);

		Color lambert = kD * albedo * INVERSE_PI;

		return (lambert + specular);
	}

	Color BRDFDirect(const Vec3f& L, const Vec3f& V, const Vec3f& N, Color albedo, float roughness, float metalness) const
	{
		float cosTheta = Max<float>(N.Dot(L), 0.0f);

		Color F0 = Color(0.04f, 0.04f, 0.04f);
		F0 = Color(
					F0.r * (1.0f - metalness) + albedo.r * metalness,
					F0.g * (1.0f - metalness) + albedo.g * metalness,
					F0.b * (1.0f - metalness) + albedo.b * metalness);
		Color F = FresnelSchlick(cosTheta, F0);

		Vec3f H = (V + L).GetNormalized();
		float NDF = DistributionGGXDirect(N, H, roughness);
		float G = GeometrySmithDirect(N, V, L, roughness);

		Color numerator = NDF * G * F;
		float denominator = 4.0f * Max<float>(N.Dot(V), 0.0f) * Max<float>(N.Dot(L), 0.0f);

		Color specular = numerator / Max<float>(denominator, 0.001f);

		Color kS = F;
		Color kD = Color(1.0f, 1.0f, 1.0f) - kS;

		kD = kD * (1.0f - metalness);

		Color lambert = kD * albedo * INVERSE_PI;

		return (lambert + specular);
	}

private:

	float DistributionGGXDirect(Vec3f N, Vec3f H, float roughness) const
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

	float GeometrySchlickGGXDirect(float NdotV, float roughness) const
	{
		float r = (roughness + 1.0f);
		float k = (r * r) / 8.0f;

		float nom = NdotV;
		float denom = NdotV * (1.0 - k) + k;

		return nom / denom;
	}

	float GeometrySmithDirect(Vec3f N, Vec3f V, Vec3f L, float roughness) const
	{
		float NdotV = Max<float>(N.Dot(V), 0.0f);
		float NdotL = Max<float>(N.Dot(L), 0.0f);
		float ggx1 = GeometrySchlickGGXDirect(NdotV, roughness);
		float ggx2 = GeometrySchlickGGXDirect(NdotL, roughness);

		return ggx1 * ggx2;
	}

	Color FresnelSchlick(float cosTheta, Color F0) const
	{
		return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0);
	}

	Color FresnelSchlickRoughness(float cosTheta, Color F0, float roughness) const
	{
		return F0 + (Color(Max<float>(1.0f - roughness, F0.r),
			Max<float>(1.0f - roughness, F0.g),
			Max<float>(1.0f - roughness, F0.b)) - F0) * pow(1.0 - cosTheta, 5.0);
	}
};