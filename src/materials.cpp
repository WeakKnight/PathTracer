#include "materials.h"
#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>
#include "raytracer.h"
#include "utils.h"
#include <unordered_map>

using namespace cy;

extern Node rootNode;
extern TexturedColor environment;
extern std::unordered_map<std::string, TextureMap> textureMap;

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

Color MtlBlinn::Shade(RayContext const &rayContext, const HitInfoContext &hInfoContext, const LightList &lights, int bounceCount) const
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
                    Color refractColor = absortionColor * refraction.GetColor() * (1.0f - Rs) * refractHitInfo.node->GetMaterial()->Shade(refractRayContext, refractHitInfoContext, lights, bounceCount - 1);
                    result += refractColor;
                }
                else
                {

                }
            }

            HitInfoContext reflectHitInfoContext;
            HitInfo& reflectHitInfo = reflectHitInfoContext.mainHitInfo;

            float reflectDistance;

            if(bounceCount > 0)
            {
                if(GenerateRayForNearestIntersection(reflectRayContext, reflectHitInfoContext, HIT_FRONT, reflectDistance))
                {
                    Color reflectColor = refraction.GetColor() * Rs * reflectHitInfo.node->GetMaterial()->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1);
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
                        Color refractColor =
                        //                            absortionColor *
                        //                            refraction *
                        (1.0f - Rs) *
                        refractHitInfo.node->GetMaterial()->Shade(refractRayContext, refractHitInfoContext, lights, bounceCount - 1);
                        result += refractColor;
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
                    * (Rs) * reflectHitInfo.node->GetMaterial()->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1);

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

                    Color refractColor = absortionColor * refraction.GetColor() * (1.0f) * reflectHitInfo.node->GetMaterial()->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1);

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
                const Material* mat = reflectHitInfo.node->GetMaterial();
                
                Color reflectColor = reflectFactor * mat->Shade(reflectRayContext, reflectHitInfoContext, lights, bounceCount - 1);
                result += reflectColor;
            }
            else
            {
                result += reflectFactor * environment.SampleEnvironment(reflectRayContext.cameraRay.dir);
            }
        }
    }
    
	Vec3f diffuseN = N;

    for(size_t index = 0; index < lights.size(); index++)
    {
        Light* light = lights[index];
        
        Color colorComing;
        
        if(light->IsAmbient())
        {
            colorComing = light->Illuminate(hInfo.p + hInfo.N * INTERSECTION_BIAS, diffuseN);
            
            Color diffuseColor = colorComing * diffuse.GetColor();
            if(diffuse.GetTexture())
            {
                if(!rayContext.hasDiff)
                {
                    diffuseColor = colorComing * diffuse.Sample(hInfo.uvw);
                }
                else
                {
                    diffuseColor = colorComing * diffuse.Sample(hInfo.uvw, hInfo.duvw);
                }
            }
            Color specularColor =
            Color::Black();
            
            result += (diffuseColor + specularColor);
        }
        else
        {
            Vec3f L = -1.0f * light->Direction(hInfo.p);
            assert(L.IsUnit());
            
            Vec3f H = (V + L).GetNormalized();
            assert(H.IsUnit());
            
            float cosTheta = L.Dot(diffuseN);
            if(cosTheta < 0.0f)
            {
                cosTheta = 0.0f;
            }
//            assert(cosBeta > - 0.00001f);
            
            if(cosTheta < 0.0f)
            {
                continue;
            }
            
            // don't shade diffuse color or specular color back side
            if(!hInfo.front)
            {
                continue;
            }
            
            Color iComing = light->Illuminate(hInfo.p + N * INTERSECTION_BIAS, diffuseN);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = colorComing * diffuse.GetColor();
            if(diffuse.GetTexture())
            {
                if(!rayContext.hasDiff)
                {
                    diffuseColor = colorComing * diffuse.Sample(hInfo.uvw);
                }
                else
                {
                    diffuseColor = colorComing * diffuse.Sample(hInfo.uvw, hInfo.duvw);
                }
            }
            
            Color specularColor = iComing * specular.GetColor() * pow(H.Dot(N), glossiness);
            
//            if(refraction.Sum() > 0.0f)
//            {
//                specularColor.SetBlack();
//            }
            
			float aoFactor = 1.0f;

			if (this->ao)
			{
				Vec3f texAo = this->ao->SampleVector(hInfo.uvw, hInfo.duvw);

				aoFactor = (texAo.x / 1.0f);
			}

            result += (aoFactor * (diffuseColor + specularColor));
        }
    }

	// indirect part


    // assert(result.Max() <= 1.0f);
    result.ClampMax();
    return result;
}




void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}
