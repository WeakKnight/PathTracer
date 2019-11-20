#include "materials.h"
#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>
#include "raytracer.h"
#include "utils.h"
#include <unordered_map>
#include "irradiancemap.h"
#include "config.h"
#include "constants.h"

using namespace cy;

extern Node rootNode;
extern TexturedColor environment;
extern std::unordered_map<std::string, TextureMap> textureMap;
extern IrradianceCacheMap irradianceCacheMap;

QuasyMonteCarloCircleSampler* MtlBlinn::normalSampler = new QuasyMonteCarloCircleSampler;



#define ENUM_RELFECTION 1
#define ENUM_REFRACTIN 2

Vec3f MtlBlinn::GenerateNormalWithGlossiness(const Vec3f& originalNormal, int type) const
{
	assert(originalNormal.IsUnit());
	
	float offset;

	if (type == ENUM_RELFECTION)
	{
		if (reflectNormalDistribution)
		{
			auto dist = reflectNormalDist;
			auto e = eng;
			offset = abs(dist(e));
		}
		else
		{
			offset = normalSampler->RandomGlossAngleFactor() * reflectionGlossiness;
		}
	}
	else
	{
		if (refractNormalDistribution)
		{
			auto dist = refractNormalDist;
			auto e = eng;
			offset = abs(dist(e));
		}
		else
		{
			offset = normalSampler->RandomGlossAngleFactor() * refractionGlossiness;
		}
	}
	
	Vec3f right;
	Vec3f forward;
	BranchlessONB(originalNormal, right, forward);

	float radius = 1.0f * sinf(offset);
	float topComponent = 1.0f * cosf(offset);

	// choose a direction from 0 to 2pi, uniformly
	float theta = normalSampler->RandomTheta();

	Vec3f result = originalNormal * topComponent + radius * sinf(theta) * right + radius * cosf(theta) * forward;
	assert(result.IsUnit());

	return result;
}

Vec3f Reflect(Vec3f I, Vec3f N)
{
    return I - 2.0f * N.Dot(I) * N;
}

void CalculateRefractDir(
                        Vec3f v,
                        Vec3f normal,
                        float n1, float n2,
                        Vec3f& refract,
                        Vec3f& reflect,
                        bool& internalReflection
                        )
{
    Vec3f NCrossV = normal.Cross(v);
    float sinTheta = NCrossV.Length();
    float sinRefract = n1 * sinTheta / n2;
    
    if(sinRefract >= 1.0f)
    {
        internalReflection = true;
    }
    else
    {
        internalReflection = false;
    }
    
    float cosRefract = sqrtf(1.0f - sinRefract * sinRefract);
    
    refract = normal.Cross(NCrossV).GetNormalized() * sinRefract + (-1.0f) * normal * cosRefract;
    reflect = Reflect(-v, normal);
}

Color MtlBlinn::Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount, int indirectLightBounce) const
{
    // ray world space
    // N world space
    // p world space
    // depth space independent
    // node space independent
    Color result = Color::Black();
    
    const HitInfo& hInfo = hInfoContext.mainHitInfo;
    const Ray& ray = rayContext.cameraRay;
    
    Vec3f V = -1.0f * ray.dir;
	V.Normalize();

	Vec3f N = hInfo.N;
	N.Normalize();

	if (this->normal)
	{
		Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
		
		N = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
		
		N.Normalize();
	}

	// direct part


	// indirect part
	Color indirectResult = IndirectLightShade(N, rayContext, hInfoContext, lights, bounceCount, indirectLightBounce);

    result += indirectResult;

	// emission part
	if (hInfo.front)
	{
		result += emission.Sample(hInfo.uvw, hInfo.duvw);
	}

    return result;
}

Color MtlBlinn::IndirectLightShade(const Vec3f& N, RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount, int indirectLightBounce) const
{
	Color result = Color::Black();

	if (indirectLightBounce <= 0)
	{
		return result;
	}

	Vec3f V = -1.0f * rayContext.cameraRay.dir;
	V.Normalize();

	const auto& p = hInfoContext.mainHitInfo.p + N * 10.0f * INTERSECTION_BIAS;
	const auto node = hInfoContext.mainHitInfo.node;

	Vec3f xBasis, yBasis;
	BranchlessONB(N, xBasis, yBasis);

	Color albedoColor = diffuse.Sample(hInfoContext.mainHitInfo.uvw, hInfoContext.mainHitInfo.duvw);
	float roughnessValue = roughness.Sample(hInfoContext.mainHitInfo.uvw, hInfoContext.mainHitInfo.duvw).r;
	if (roughnessValue <= 0.0f)
	{
		roughnessValue = 0.001f;
	}
	float metalnessValue = metalness.Sample(hInfoContext.mainHitInfo.uvw, hInfoContext.mainHitInfo.duvw).r;
	int count = 1;

	for (int i = 0; i < count; i++)
	{
		Color indirectLightIntencity = Color::Black();
		//===========================================
		float probabilityGGX;
		Vec3f randomGGXWeighted = ImportanceSampleGGX(roughnessValue, probabilityGGX);
		Vec3f rayDirGGXWeighted = randomGGXWeighted.z * N + randomGGXWeighted.x * xBasis + randomGGXWeighted.y * yBasis;

		Ray indirectRayGGXWeighted(p, rayDirGGXWeighted);
		indirectRayGGXWeighted.Normalize();

		float NDotLGGXWeighted = Max<float>(N.Dot(indirectRayGGXWeighted.dir), 0.001f);
		// =================================
		Vec3f randomCosineWeighted = CosineWeightedRandomPointOnHemiSphere();
		Vec3f rayDirCosineWeighted = randomCosineWeighted.z * N + randomCosineWeighted.x * xBasis + randomCosineWeighted.y * yBasis;
		Ray indirectRayCosineWeighted(p, rayDirCosineWeighted);
		indirectRayCosineWeighted.Normalize();

		float NDotLCosineWeighted = Max<float>(N.Dot(indirectRayCosineWeighted.dir), 0.001f);
		float probabilityCosine = NDotLCosineWeighted * INVERSE_PI;
		// ==================================
		// MIS
		Vec3f rayDir;
		Ray indirectRay;
		float cosTheta;
		float probability;

		int misIndex = MIS2(probabilityCosine, probabilityGGX);
		if (misIndex == 0)
		{
			rayDir = indirectRayCosineWeighted.dir;
			indirectRay = indirectRayCosineWeighted;
			cosTheta = NDotLCosineWeighted;
			probability = probabilityCosine;
		}
		else 
		{
			rayDir = indirectRayGGXWeighted.dir;
			indirectRay = indirectRayGGXWeighted;
			cosTheta = NDotLGGXWeighted;
			probability = probabilityGGX;
		}
		//=========================================

		RayContext indirectRayContext;
		indirectRayContext.cameraRay = indirectRay;
		indirectRayContext.rightRay = indirectRay;
		indirectRayContext.topRay = indirectRay;
		indirectRayContext.hasDiff = false;

		HitInfoContext indirectHitInfoContext;

		bool hit = TraceNode(indirectHitInfoContext, indirectRayContext, &rootNode, HIT_FRONT_AND_BACK);
		if (hit)
		{
			auto inDirectNode = indirectHitInfoContext.mainHitInfo.node;
			float fallFactor = LightFallOffFactor(p, indirectHitInfoContext.mainHitInfo.p);
			// float fallFactor = 1.0f;
			if (!LightFallOff)
			{
				fallFactor = 1.0f;
			}

			indirectLightIntencity = fallFactor * indirectHitInfoContext.mainHitInfo.mtl->Shade(indirectRayContext, indirectHitInfoContext, lights, bounceCount, indirectLightBounce - 1);
		}
		else
		{
			indirectLightIntencity = environment.SampleEnvironment(indirectRayContext.cameraRay.dir);
		}

		Color brdfTerm = brdf.BRDF(indirectRay.dir, V, N, albedoColor, roughnessValue, metalnessValue);
		if (isinf(brdfTerm.Sum()))
		{
			brdfTerm = Color::White();
		}

		Color indirectShading = ((1.0f/ (float)count) * cosTheta * indirectLightIntencity * brdfTerm / probability);
		if (isnan(indirectShading.Sum()))
		{
			indirectShading = Color::Black();
		}
		result += indirectShading;
	}

	return result;
}

void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}
