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

#define INTERSECTION_BIAS 0.0001f

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
    assert(V.IsUnit());
    
	Vec3f N = hInfo.N;
	assert(N.IsUnit());

	if (this->normal)
	{
		Vec3f texNormal = this->normal->SampleVector(hInfo.uvw, hInfo.duvw);
		
		// Vec3f right;
		// Vec3f forward;
		// branchlessONB(N, right, forward);

		// N = hInfo.N * texNormal.z + right * texNormal.y + forward * texNormal.x;
		N = hInfo.N * texNormal.z + hInfo.Bitangent.GetNormalized() * texNormal.y + hInfo.Tangent.GetNormalized() * texNormal.x;
		
		N.Normalize();
	}
    
    float cosBeta = V.Dot(N);
    if(cosBeta < 0.0f)
    {
        cosBeta = 0.0f;
    }
    
    // has refraction
    if(refraction.GetColor().Sum() > 0.0f)
    {
        // Schlicks Approximation
        float Rs = 0.0f;
        float n1 = 0.0f;
        float n2 = 0.0f;

        if(hInfo.front)
        {
            n1 = 1.0f;
            n2 = ior;
        }
        else
        {
            n1 = ior;
            n2 = 1.0f;
        }

        float Rs0 = powf((n1 - n2)/(n1 + n2), 2.0f);
        Rs = Rs0 + ((1.0f - Rs0) * powf(1.0f - max(0.0f, cosBeta), 5.0f));

        bool hasInternalReflection = false;

        Vec3f inRefractDir;
        Vec3f inRelectDir;

		Vec3f refractN = GenerateNormalWithGlossiness(N, ENUM_REFRACTIN);

        CalculateRefractDir(V, refractN, n1, n2, inRefractDir, inRelectDir, hasInternalReflection);

        Ray inReflectRay;
        inReflectRay.dir = inRelectDir;
        inReflectRay.p = hInfo.p + N * INTERSECTION_BIAS;

        Ray inRefractRay;
        inRefractRay.dir = inRefractDir;
        inRefractRay.p = hInfo.p + (-2.0f) * N * INTERSECTION_BIAS;

        RayContext reflectRayContext;
        reflectRayContext.cameraRay = inReflectRay;
        reflectRayContext.rightRay = inReflectRay;
        reflectRayContext.topRay = inReflectRay;
		reflectRayContext.hasDiff = false;
        reflectRayContext.delta = RAY_DIFF_DELTA;

        RayContext refractRayContext;
        refractRayContext.cameraRay = inRefractRay;
        refractRayContext.rightRay = inRefractRay;
        refractRayContext.topRay = inRefractRay;
		reflectRayContext.hasDiff = false;
        refractRayContext.delta = RAY_DIFF_DELTA;

        // from air, has absortion. no internal reflection
        if(hInfo.front)
        {
            assert(!hasInternalReflection);

            HitInfoContext refractHitInfoContext;
            HitInfo& refractHitInfo = refractHitInfoContext.mainHitInfo;

            float distance;

            if(bounceCount > 0)
            {
                if(GenerateRayForNearestIntersection(refractRayContext, refractHitInfoContext, HIT_BACK, distance))
                {
                    Color absortionColor = Color(
                                                 powf(M_E, -1.0f * distance * absorption.r)
                                                 , powf(M_E, -1.0f * distance * absorption.g)
                                                 , powf(M_E, -1.0f * distance * absorption.b));
                    Color refractColor = absortionColor * refraction.GetColor() * (1.0f - Rs) * refractHitInfo.mtl->Shade(refractRayContext, refractHitInfoContext, lights, bounceCount - 1, indirectLightBounce);
                    result += refractColor;
                }
            }

            HitInfoContext reflectHitInfoContext;
            HitInfo& reflectHitInfo = reflectHitInfoContext.mainHitInfo;

            float reflectDistance;

            if(bounceCount > 0)
            {
                if(GenerateRayForNearestIntersection(reflectRayContext, reflectHitInfoContext, HIT_FRONT, reflectDistance))
                {
                    Color reflectColor = refraction.GetColor() * Rs * reflectHitInfo.mtl->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1, indirectLightBounce);
                    result += reflectColor;
                }
                else
                {
                    result += refraction.GetColor() * Rs * environment.SampleEnvironment(inReflectRay.dir);
                }
            }
        }
        // into air, no absortion
        else
        {
            if(!hasInternalReflection)
            {
                HitInfoContext refractHitInfoContext;
                HitInfo& refractHitInfo = refractHitInfoContext.mainHitInfo;

                float distance;

                if(bounceCount > 0)
                {
                    if(GenerateRayForNearestIntersection(refractRayContext, refractHitInfoContext, HIT_FRONT, distance))
                    {
						float relractFactor = 1.0f - Rs;
						if (refractHitInfo.mtl != nullptr)
						{
							Color refractShadeResult = refractHitInfo.mtl->Shade(refractRayContext, refractHitInfoContext, lights, bounceCount - 1, indirectLightBounce);
							Color refractColor = relractFactor * refractShadeResult;
							//                            absortionColor *
							//                            refraction *
							result += refractColor;
						}
                    }
                    else
                    {
                        result += (1.0f - Rs) * environment.SampleEnvironment(inRefractRay.dir);
                    }
                }

                HitInfoContext reflectHitInfoContext;
                HitInfo& reflectHitInfo = reflectHitInfoContext.mainHitInfo;

                float reflectDistance;

                if(bounceCount > 0 && GenerateRayForNearestIntersection(reflectRayContext, reflectHitInfoContext, HIT_BACK, reflectDistance))
                {
                    Color absortionColor = Color(
                                                 powf(M_E, -1.0f * reflectDistance * absorption.r)
                                                 , powf(M_E, -1.0f * reflectDistance * absorption.g)
                                                 , powf(M_E, -1.0f * reflectDistance * absorption.b));

                    Color refractColor = absortionColor
                    //                            * refraction
                    * (Rs) * reflectHitInfo.mtl->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1, indirectLightBounce);

                    result += refractColor;
                }
            }
            // totaly reflection
            else
            {
                HitInfoContext reflectHitInfoContext;
                HitInfo& reflectHitInfo = reflectHitInfoContext.mainHitInfo;

                float distance;

                if(bounceCount > 0 && GenerateRayForNearestIntersection(reflectRayContext, reflectHitInfoContext, HIT_BACK, distance))
                {
                    Color absortionColor = Color(
                                                 powf(M_E, -1.0f * distance * absorption.r)
                                                 , powf(M_E, -1.0f * distance * absorption.g)
                                                 , powf(M_E, -1.0f * distance * absorption.b));

                    Color refractColor = absortionColor * refraction.GetColor() * (1.0f) * reflectHitInfo.mtl->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1, indirectLightBounce);

                    result += refractColor;
                }
            }
        }
    }
    
    Color reflectFactor = this->reflection.GetColor();
    if(this->reflection.GetTexture())
    {
        reflectFactor = this->reflection.Sample(hInfo.uvw, hInfo.duvw);
    }
    
    // has reflection
    if(reflectFactor.Sum() > 0.0f && bounceCount > 0)
    {
        // only consider front reflect
        if(hInfo.front)
        {
			Vec3f reflectN = GenerateNormalWithGlossiness(N, ENUM_RELFECTION);

            // Generate Reflect Ray Check Target
            Vec3f R = Reflect(ray.dir, reflectN);
            
            assert(R.IsUnit());
            assert(reflectN.IsUnit());
            assert(ray.dir.IsUnit());
            
            // world space
            Ray reflectRay;
            reflectRay.dir = R;
            reflectRay.p = hInfo.p;
            
            RayContext reflectRayContext;
            reflectRayContext.cameraRay = reflectRay;
            reflectRayContext.rightRay = reflectRay;
            reflectRayContext.topRay = reflectRay;
            reflectRayContext.delta = RAY_DIFF_DELTA;
			reflectRayContext.hasDiff = false;
            
            HitInfoContext reflectHitInfoContext;
            HitInfo& reflectHitInfo = reflectHitInfoContext.mainHitInfo;
            
            float transDistance = 0.0f;
            
            // detect front intersection for reflection shading
            if(GenerateRayForNearestIntersection(reflectRayContext, reflectHitInfoContext, HIT_FRONT, transDistance))
            {
                const Material* mat = reflectHitInfo.mtl;
                
                Color reflectColor = reflectFactor * mat->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1, indirectLightBounce);
                result += reflectColor;
            }
            else
            {
                result += reflectFactor * environment.SampleEnvironment(reflectRayContext.cameraRay.dir);
            }
        }
    }

	// direct part
	Color directResult = DirectLightShade(rayContext, hInfoContext, lights);
	result += directResult;

	// indirect part
	Color indirectResult;
	if (IrradianceCache)
	{
		indirectResult = irradianceCacheMap.Sample(hInfoContext.screenX, hInfoContext.screenY).c;
	}
	else
	{
		indirectResult = IndirectLightShade(rayContext, hInfoContext, lights, bounceCount, indirectLightBounce);
	}

    result += indirectResult;

	// emission part
	if (rayContext.hasDiff)
	{
		result += emission.Sample(hInfo.uvw, hInfo.duvw);
	}
	else
	{
		result += emission.Sample(hInfo.uvw);
	}

    assert(!isnan(result.r + result.g + result.b));
    return result;
}

Color MtlBlinn::DirectLightShade(RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights) const
{
	Color result = Color::Black();

	// don't shade diffuse color or specular color back side
	if (!hInfoContext.mainHitInfo.front)
	{
		return result;
	}

	const HitInfo& hInfo = hInfoContext.mainHitInfo;

	Vec3f V = -1.0f * rayContext.cameraRay.dir;
	auto N = hInfoContext.mainHitInfo.N.GetNormalized();
	const Vec3f& p = hInfoContext.mainHitInfo.p;

	float inverseProbability;
	Light* light = LiForDirect(p, N, lights, inverseProbability);

	Vec3f L = -1.0f * light->Direction(p);
	assert(L.IsUnit());

	float cosTheta = L.Dot(N);
	if (cosTheta < 0.0f)
	{
		cosTheta = 0.0f;
	}

	Color flexForSpecular = inverseProbability * light->Illuminate(p + N * INTERSECTION_BIAS, N);
	Color flex = flexForSpecular * cosTheta;

	Color diffuseColor = flex * Lambert(L, V, N, p, hInfo);
	Color specularColor = flexForSpecular * Specular(L, V, N, p, hInfo);

	float aoFactor = 1.0f;

	if (this->ao)
	{
		Vec3f texAo = this->ao->SampleVector(hInfo.uvw, hInfo.duvw);
		aoFactor = (texAo.x / 1.0f);
	}

	result = aoFactor * (diffuseColor + specularColor);

	return result;
}

Color MtlBlinn::IndirectLightShade(RayContext const& rayContext, const HitInfoContext& hInfoContext, const LightList& lights, int bounceCount, int indirectLightBounce) const
{
	Color result = Color::Black();

	if (indirectLightBounce <= 0)
	{
		return result;
	}

	Vec3f V = -1.0f * rayContext.cameraRay.dir;

	auto N = hInfoContext.mainHitInfo.N.GetNormalized();
	const auto& p = hInfoContext.mainHitInfo.p + N * 10.0f * INTERSECTION_BIAS;
	const auto node = hInfoContext.mainHitInfo.node;

	Vec3f xBasis, yBasis;
	BranchlessONB(N, xBasis, yBasis);

	// float constantFactor = (2.0f * Pi<float>()) / (float)IndirectLightSampleCount;
    float constantFactor = 1.0f * PI;
    
	QuasyMonteCarloHemiSphereSampler sampler;

	Color indirectLightIntencity = Color::Black();

	Vec3f randomPoint = sampler.CosineWeightedSample();
	// Vec3f randomPoint = UniformRandomPointOnHemiSphere();
	Vec3f rayDir = randomPoint.z * N + randomPoint.x * xBasis + randomPoint.y * yBasis;
	Ray indirectRay(p, rayDir);
	indirectRay.Normalize();
        
	Vec3f H = (V + indirectRay.dir).GetNormalized();

    assert(!isnan(rayDir.x + rayDir.y + rayDir.z));

	RayContext indirectRayContext;
	indirectRayContext.cameraRay = indirectRay;
	indirectRayContext.rightRay = indirectRay;
	indirectRayContext.topRay = indirectRay;
	indirectRayContext.hasDiff = false;

	HitInfoContext indirectHitInfoContext;

	bool hit = TraceNode(indirectHitInfoContext, indirectRayContext, &rootNode, HIT_FRONT_AND_BACK);
	if (hit)
	{
		float distance = indirectHitInfoContext.mainHitInfo.z;
		float distanceSquare = distance * distance;

		auto inDirectNode = indirectHitInfoContext.mainHitInfo.node;
		float fallFactor = 1.0f / (1.0f + 0.09f * distance + 0.032f * distanceSquare);
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

	float cosTheta = N.Dot(indirectRay.dir);
	if (cosTheta < 0.0f)
	{
		cosTheta = 0.0f;
	}

	Color flexForSpecular = indirectLightIntencity;
	Color flex = flexForSpecular * cosTheta;

	Color diffuseColor = flex * Lambert(indirectRay.dir, V, N, p, hInfoContext.mainHitInfo);
	Color specularColor = flexForSpecular * Specular(indirectRay.dir, V, N, p, hInfoContext.mainHitInfo);

	result =  (constantFactor * (diffuseColor + specularColor));
	
	return result;
}

Color MtlBlinn::Lambert(const Vec3f& wi, const Vec3f& wo, const Vec3f& n, const Vec3f& x, const HitInfo& hitInfo) const
{
	return diffuse.Sample(hitInfo.uvw, hitInfo.duvw) * INVERSE_PI;
}
Color MtlBlinn::Specular(const Vec3f& wi, const Vec3f& wo, const Vec3f& n, const Vec3f& x, const HitInfo& hitInfo) const
{
	Vec3f H = (wi + wo).GetNormalized();
	float HDotN = H.Dot(n);
	if (HDotN < 0.0f)
	{
		HDotN = 0.0f;
	}

	float factor = pow(HDotN, glossiness) * (glossiness + 2.0f) * INVERSE_PI * 0.5f;
	return specular.Sample(hitInfo.uvw, hitInfo.duvw) * H.Dot(n) * factor;
}

float MtlBlinn::NDF(const Vec3f& n, const Vec3f& h, float roughness) const
{
	return 0.0f;
}

float MtlBlinn::GeometrySmith(const Vec3f& n, const Vec3f& v, const Vec3f& l, float k) const
{
	return 0.0f;
}

void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}

Light* MtlBlinn::LiForDirect(const Vec3f& x, const Vec3f& n, const LightList& lights, float& inverseProbability) const
{
	// Importance Sampling For Lights
	float totalIntensity = 0.0f;

	std::vector<float> CDF;

	CDF.push_back(0.0f);

	for (int i = 0; i < lights.size(); i++)
	{
		auto light = lights[i];
		totalIntensity += light->GetFallOffIntensity(x).Max();

		CDF.push_back(totalIntensity);
	}

	float sample = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * totalIntensity;

	for (int i = 0; i < lights.size(); i++)
	{
		auto light = lights[i];

		float a = CDF[i];
		float b = CDF[(size_t)i + 1];

		if (sample >= a && sample <= b)
		{
			// fall in this segment
			auto intensity = light->GetFallOffIntensity(x);
			inverseProbability = totalIntensity / intensity.Max();
			return light;
		}
	}

	spdlog::error("Shoud not be there");
	return nullptr;
}