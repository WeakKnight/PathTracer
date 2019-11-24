#pragma once

#include "cyVector.h"
#include "cyColor.h"
#include "utils.h"
#include "constants.h"

float RandomFrom0To1()
{
	return (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
}

float sqr(float f) 
{
	return f * f;
}

float min(float a, float b)
{
	return a > b ? b : a;
}

float max(float a, float b)
{
	return a < b ? b : a;
}

float clamp(float target, float left, float right)
{
	target = min(right, target);
	target = max(left, target);
	return target;
}

float mix(float a, float b, float ratio)
{
	return a* (1.0f - ratio) + b * ratio;
}

Vec3f mix(const Vec3f& a, const Vec3f& b, float ratio)
{
	return a * (1.0f - ratio) + b * ratio;
}

Vec3f reflect(const Vec3f& I, const Vec3f& N)
{
	return I - 2.0f * N.Dot(I) * N;
}

Vec3f CosineSampleHemisphere(float u1, float u2) {
	Vec3f dir;
	float r = sqrt(u1);
	float phi = TWO_PI * u2;
	dir.x = r * cos(phi);
	dir.y = r * sin(phi);
	dir.z = sqrt(max(0.0f, 1.0f - dir.x * dir.x - dir.y * dir.y));
	return dir;
}

float GTR1(float NdotH, float a) 
{
	if (a >= 1.0f)
	{
		return INV_PI;
	}
	float a2 = a * a;
	float t = 1.0f + (a2 - 1.0f) * NdotH * NdotH;
	return (a2 - 1.0f) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a) 
{
	float a2 = a * a;
	float t = 1.0f + (a2 - 1.0f) * NdotH * NdotH;
	return a2 / (PI * t * t);
}

float SmithGGX_G(float NdotV, float a) 
{
	float a2 = a * a;
	float b = NdotV * NdotV;
	return 1.0f / (NdotV + sqrt(a2 + b - a2 * b));
}

float SchlickFresnelReflectance(float u) 
{
	float m = clamp(1.0 - u, 0.0, 1.0);
	float m2 = m * m;
	return m2 * m2 * m;
}

class DisneyShadingInfo
{
public:
	Vec3f baseColor;
	float metallic = 0.0f;
	float specular = 0.0f;
	//float anisotropy = 0.0f;
	float roughness = 0.0f;
	float specularTint = 0.0f;
	float sheenTint = 0.0f;
	float sheen = 0.0f;
	float clearcoatGloss = 0.0f;
	float clearcoat = 0.0f;
	float subsurface = 0.0f;
	float csw;

	void Clamp()
	{
		metallic = clamp(metallic, 0.001f, 0.999f);
		specular = clamp(specular, 0.001f, 0.999f);
		roughness = clamp(roughness, 0.001f, 0.999f);
		//anisotropy = clamp(anisotropy, 0.001f, 0.999f);
		specularTint = clamp(specularTint, 0.001f, 0.999f);
		sheenTint = clamp(sheenTint, 0.001f, 0.999f);
		sheen = clamp(sheen, 0.001f, 0.999f);
		clearcoatGloss = clamp(clearcoatGloss, 0.001f, 0.999f);
		clearcoat = clamp(clearcoat, 0.001f, 0.999f);
		subsurface = clamp(subsurface, 0.001f, 0.999f);
	}

	void InitCSW()
	{
		const Vec3f cd_lin = baseColor;
		const float cd_lum = cd_lin.Dot(Vec3f(0.3f, 0.6f, 0.1f));
		const Vec3f c_tint = cd_lum > 0.0f ? (cd_lin / cd_lum) : Vec3f(1.0f);
		const Vec3f c_spec0 = mix((1.0f - specular * 0.3f) * mix(Vec3f(1), c_tint, specularTint), cd_lin, metallic);
		const float cs_lum = c_spec0.Dot(Vec3f(0.3f, 0.6f, 0.1f));
		const float cs_w = cs_lum / (cs_lum + (1.0f - metallic) * cd_lum);
		csw = cs_w;
	}
};

class BrdfDisney
{
public:
	float DisneyPdf(DisneyShadingInfo& shading, const float NdotH, const float NdotL, const float HdotL)
	{
		const float d_pdf = NdotL * (1.0f / PI);
		const float r_pdf = GTR2(NdotH, max(0.001f, max(0.001f, shading.roughness))) * NdotH / (4.0f * HdotL);
		const float c_pdf = GTR1(NdotH, mix(0.1f, 0.001f, mix(0.1f, 0.001f, shading.clearcoatGloss))) * NdotH / (4.0f * HdotL);

		const float cs_w = shading.csw;

		float result = c_pdf * shading.clearcoat + (1.0f - shading.clearcoat) * (cs_w * r_pdf + (1.0f - cs_w) * d_pdf);
		if (isnan(result))
		{
			int a = 1;
		}
		if (isinf(result))
		{
			int a = 1;
		}
		return result;
	}

	Vec3f DisneyEval(DisneyShadingInfo& shading, float NdotL,  const float NdotV,  const float NdotH,  const float HdotL) {
		if (NdotL <= 0.0f || NdotV <= 0.0f)
		{
			return Vec3f(0.0f);
		}

		const Vec3f cd_lin = shading.baseColor;
		const float cd_lum = cd_lin.Dot(Vec3f(0.3f, 0.6f, 0.1f));
		const Vec3f c_tint = cd_lum > 0.0f ? (cd_lin / cd_lum) : Vec3f(1.0f);
		const Vec3f c_spec0 = mix(shading.specular * 0.3f * mix(Vec3f(1.0f), c_tint, shading.specularTint), cd_lin, shading.metallic);
		const Vec3f c_sheen = mix(Vec3f(1.0f), c_tint, shading.sheenTint);

		// Diffuse fresnel - go from 1 at normal incidence to 0.5 at grazing
		// and mix in diffuse retro-reflection based on roughness
		const float f_wo = SchlickFresnelReflectance(NdotV);
		const float f_wi = SchlickFresnelReflectance(NdotL);

		const float fd90 = 0.5f + 2.0f * HdotL * HdotL * shading.roughness;
		const float fd = mix(1.0f, fd90, f_wo) * mix(1.0f, fd90, f_wi);

		// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
		// 1.25 scale is used to (roughly) preserve albedo
		// fss90 used to "flatten" retroreflection based on roughness
		const float fss90 = HdotL * HdotL * shading.roughness;
		const float fss = mix(1.0f, fss90, f_wo) * mix(1.0f, fss90, f_wi);
		const float ss = 1.25f * (fss * (1.0f / (NdotV + NdotL) - 0.5f) + 0.5f);

		// Specular
		//float ax = max(0.001, roughness * roughness * (1.0 + anisotropy));
		//float ay = max(0.001, roughness * roughness * (1.0 - anisotropy));
		const float ro = max(0.001f, shading.roughness);
		const float ds = GTR2(NdotH, ro);
		const float fh = SchlickFresnelReflectance(HdotL);
		const Vec3f fs = mix(c_spec0, Vec3f(1.0f), fh);

		float gs = 0.0f;
		const float ro2 = sqr(shading.roughness * 0.5f + 0.5f);
		gs = SmithGGX_G(NdotV, ro2);
		gs *= SmithGGX_G(NdotL, ro2);

		// Sheen
		const Vec3f f_sheen = fh * shading.sheen * c_sheen;

		// clearcoat (ior = 1.5 -> F0 = 0.04)
		const float dr = GTR1(NdotH, mix(0.1f, 0.001f, shading.clearcoatGloss));
		const float fr = mix(0.04f, 1.0f, fh);
		const float gr = SmithGGX_G(NdotV, 0.25f) * SmithGGX_G(NdotL, 0.25f);

		const Vec3f f = ((1.0f / PI) * mix(fd, ss, shading.subsurface) * cd_lin + f_sheen) * (1.0f - shading.metallic) + gs * fs * ds + (0.25f * shading.clearcoat) * gr * fr * dr;
		if (isnan(f.Sum()))
		{
			int a = 1;
		}
		if (isinf(f.Sum()))
		{
			int a = 1;
		}
		return f * NdotL;
	}

	Vec3f DisneySample(DisneyShadingInfo& shading, const Vec3f& V, const Vec3f& N) 
	{
		float r1 = RandomFrom0To1();
		float r2 = RandomFrom0To1();

		const Vec3f U = abs(N.z) < (1.0f - EPSILON) ? Vec3f(0.0f, 0.0f, 1.0f) : Vec3f(1.0f, 0.0f, 0.0f);
		const Vec3f T = U.Cross(N).GetNormalized();
		const Vec3f B = N.Cross(T);

		// clearcoat
		if (r1 < shading.clearcoat) {
			r1 /= (shading.clearcoat);
			const float a = mix(0.1f, 0.001f, shading.clearcoatGloss);
			const float cosTheta = sqrt((1.0f - pow(a * a, 1.0f - r2)) / (1.0f - a * a));
			const float sinTheta = sqrt(max(0.0f, 1.0f - (cosTheta * cosTheta)));
			const float phi = r1 * TWO_PI;
			Vec3f H = Vec3f(
				cos(phi) * sinTheta,
				sin(phi) * sinTheta,
				cosTheta
			).GetNormalized();

			H = H.x * T + H.y * B + H.z * N;
			if (H.Dot(V) <= 0.0f)
			{
				H = H * -1.0f;
			}
			return reflect(-V, H);
		}
		r1 -= (shading.clearcoat);
		r1 /= (1.0f - shading.clearcoat);

		// specular
		if (r2 < shading.csw) 
		{
			r2 /= shading.csw;
			const float a = max(0.001f, shading.roughness);
			const float cosTheta = sqrt((1.0f - r2) / (1.0f + (a * a - 1.0f) * r2));
			const float sinTheta = sqrt(max(0.0f, 1.0f - (cosTheta * cosTheta)));
			const float phi = r1 * TWO_PI;
			Vec3f H = Vec3f(
				cos(phi) * sinTheta,
				sin(phi) * sinTheta,
				cosTheta
			).GetNormalized();

			H = H.x * T + H.y * B + H.z * N;
			if (H.Dot(V) <= 0.0f)
			{
				H = H * -1.0f;
			}
			return reflect(-V, H);
		}
		// diffuse
		r2 -= shading.csw;
		r2 /= (1.0f - shading.csw);
		const Vec3f H = CosineSampleHemisphere(r1, r2);
		return T * H.x + B * H.y + N * H.z;
	}
};