#include "materials.h"
#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>
#include "raytracer.h"

using namespace cy;

extern Node rootNode;

#define INTERSECTION_BIAS 0.0001f

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

Color MtlBlinn::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const
{
    // ray world space
    // N world space
    // p world space
    // depth space independent
    // node space independent
    Color result = Color::Black();
    
    for(size_t index = 0; index < lights.size(); index++)
    {
        Light* light = lights[index];
        
        Color colorComing;
        
        if(light->IsAmbient())
        {
            colorComing = light->Illuminate(hInfo.p + hInfo.N * INTERSECTION_BIAS, hInfo.N);
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor =
            Color::Black();
            
            result += (diffuseColor + specularColor);
        }
        else
        {
            Vec3f V = -1.0f * ray.dir;
            assert(V.IsUnit());
            
            Vec3f L = -1.0f * light->Direction(hInfo.p);
            assert(L.IsUnit());
            
            Vec3f N = hInfo.N;
            assert(N.IsUnit());
            
            Vec3f H = (V + L).GetNormalized();
            assert(H.IsUnit());
            
            float cosTheta = L.Dot(N);
            
            float cosBeta = V.Dot(N);
            assert(cosBeta > - 0.00001f);
            
            // has refraction
            if(refraction.Sum() > 0.0f)
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
                
                CalculateRefractDir(V, N, n1, n2, inRefractDir, inRelectDir, hasInternalReflection);
                
                Ray inReflectRay;
                inReflectRay.dir = inRelectDir;
                inReflectRay.p = hInfo.p + N * INTERSECTION_BIAS;
                
                Ray inRefractRay;
                inRefractRay.dir = inRefractDir;
                inRefractRay.p = hInfo.p + (-1.0f) * N * INTERSECTION_BIAS;
                
                // from air, has absortion. no internal reflection
                if(hInfo.front)
                {
                    assert(!hasInternalReflection);
                    
                    HitInfo refractHitInfo;
                    float distance;
                    
                    if(bounceCount > 0 && GenerateRayForNearestIntersection(inRefractRay, refractHitInfo, HIT_BACK, distance))
                    {
                        Color absortionColor = Color(
                                                       powf(M_E, -1.0f * distance * absorption.r)
                                                     , powf(M_E, -1.0f * distance * absorption.g)
                                                     , powf(M_E, -1.0f * distance * absorption.b));
                        Color refractColor = absortionColor * refraction * (1.0f - Rs) * refractHitInfo.node->GetMaterial()->Shade(inRefractRay, refractHitInfo, lights, bounceCount - 1);
                        result += refractColor;
                    }
                    
                    HitInfo reflectHitInfo;
                    float reflectDistance;
                    
                    if(bounceCount > 0 && GenerateRayForNearestIntersection(inReflectRay, reflectHitInfo, HIT_FRONT, reflectDistance))
                    {
                        Color reflectColor = Rs * reflectHitInfo.node->GetMaterial()->Shade(inReflectRay, reflectHitInfo, lights, bounceCount - 1);
                        result += reflectColor;
                    }
                }
                // into air, no absortion
                else
                {
                    if(!hasInternalReflection)
                    {
                        HitInfo refractHitInfo;
                        float distance;
                        
                        if(bounceCount > 0 && GenerateRayForNearestIntersection(inRefractRay, refractHitInfo, HIT_FRONT, distance))
                        {
//                            Color absortionColor = Color(
//                                                         powf(M_E, -1.0f * distance * absorption.r)
//                                                         , powf(M_E, -1.0f * distance * absorption.g)
//                                                         , powf(M_E, -1.0f * distance * absorption.b));
                            
                            Color refractColor =
//                            absortionColor *
                            refraction *
                            (1.0f - Rs) *
                            refractHitInfo.node->GetMaterial()->Shade(inRefractRay, refractHitInfo, lights, bounceCount - 1);
                            result += refractColor;
                        }
                        
                        HitInfo reflectHitInfo;
                        float reflectDistance;
                        
                        if(bounceCount > 0 && GenerateRayForNearestIntersection(inReflectRay, reflectHitInfo, HIT_BACK, reflectDistance))
                        {
                            Color absortionColor = Color(
                                                         powf(M_E, -1.0f * reflectDistance * absorption.r)
                                                         , powf(M_E, -1.0f * reflectDistance * absorption.g)
                                                         , powf(M_E, -1.0f * reflectDistance * absorption.b));
                            
                            Color refractColor = absortionColor * refraction * (Rs) * reflectHitInfo.node->GetMaterial()->Shade(inReflectRay, reflectHitInfo, lights, bounceCount - 1);
                            
                            result += refractColor;
                        }
                    }
                    // totaly reflection
                    else
                    {
                        HitInfo reflectHitInfo;
                        float distance;
                        
                        if(bounceCount > 0 && GenerateRayForNearestIntersection(inReflectRay, reflectHitInfo, HIT_BACK, distance))
                        {
                            Color absortionColor = Color(
                                                         powf(M_E, -1.0f * distance * absorption.r)
                                                         , powf(M_E, -1.0f * distance * absorption.g)
                                                         , powf(M_E, -1.0f * distance * absorption.b));
                            
                            Color refractColor = absortionColor * refraction * (1.0f) * reflectHitInfo.node->GetMaterial()->Shade(inReflectRay, reflectHitInfo, lights, bounceCount - 1);
                            
                            result += refractColor;
                        }
                    }
                }
            }
            
            // has reflection
            if(reflection.Sum() > 0.0f && bounceCount > 0)
            {
                // only consider front reflect
                if(hInfo.front)
                {
                    // Generate Reflect Ray Check Target
                    Vec3f R = Reflect(ray.dir, N);
                    assert(R.IsUnit());
                    
                    // world space
                    Ray reflectRay;
                    reflectRay.dir = R;
                    reflectRay.p = hInfo.p + N * INTERSECTION_BIAS;
                    
                    HitInfo reflectHitInfo;
                    float transDistance = 0.0f;
                    // detect front intersection for reflection shading
                    if(GenerateRayForNearestIntersection(reflectRay, reflectHitInfo, HIT_FRONT, transDistance))
                    {
                        const Material* mat = reflectHitInfo.node->GetMaterial();
                        
                        Color reflectColor = this->reflection * mat->Shade(reflectRay, reflectHitInfo, lights, bounceCount - 1);
                        result += reflectColor;
                    }
                }
            }
            
            if(cosTheta < 0.0f)
            {
                continue;
            }
            
            // don't shade diffuse color or specular color back side
            if(!hInfo.front)
            {
                continue;
            }
            
            Color iComing = light->Illuminate(hInfo.p + N * INTERSECTION_BIAS, N);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = Color(colorComing * diffuse);
            Color specularColor = iComing * specular * pow(H.Dot(N), glossiness);
            
            result += (diffuseColor + specularColor);
        }
    }
    result.ClampMax();
    return result;
}




void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}

Color MtlPhong::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights, int bounceCount) const
{
    // ray world space
    // N obj space need transform
    // p obj space need transform
    // depth space independent
    // node space independent
    Color result = Color::Black();
    
    for(size_t index = 0; index < lights.size(); index++)
    {
        Light* light = lights[index];
        
        Color colorComing;
        Vec3f lightDir = -1.0f * light->Direction(hInfo.p);
        
        Vec3f R = Reflect(lightDir, hInfo.N);
        Vec3f V = -1.0f * ray.dir.GetNormalized();
        
        float cosTheta = lightDir.Dot(hInfo.N);
        
        if(light->IsAmbient())
        {
            colorComing = light->Illuminate(hInfo.p, hInfo.N);
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor =
            Color::Black();
//            colorComing * specular * pow(V.Dot(R), glossiness);
            // / cosTheta;
            
            result += (diffuseColor + specularColor);
        }
        else
        {
            if(cosTheta < 0)
            {
                continue;
            }
            
            Color iComing = light->Illuminate(hInfo.p, hInfo.N);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor = iComing * specular * pow(V.Dot(R), glossiness);
            // / cosTheta;
            
            result += (diffuseColor + specularColor);
        }
    }
    
    result.ClampMax();
    return result;
}

void MtlPhong::SetViewportMaterial(int subMtlID) const
{
}
