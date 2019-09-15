#include "materials.h"
#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>
#include "raytracer.h"

using namespace cy;

#define INTERSECTION_BIAS 0.0001f

Vec3f Reflect(Vec3f I, Vec3f N)
{
    return I - 2.0f * N.Dot(I) * N;
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
            Vec3f V = -1.0f * ray.dir.GetNormalized();
            assert(V.IsUnit());
            
            Vec3f L = -1.0f * light->Direction(hInfo.p);
            assert(L.IsUnit());
            
            Vec3f N = hInfo.N;
            assert(N.IsUnit());
            
            Vec3f H = (V + L).GetNormalized();
            assert(H.IsUnit());
            
            float cosTheta = L.Dot(N);
            
            // Schlicks Approximation
            float Rs = 0.0f;
            
            // has refraction
            if(refraction.Sum() > 0.0f)
            {
                float n1 = 0.0f;
                float n2 = 0.0f;
                
                // currently ignore the situation from one refraction material into another refraction material
                // from air to this
                if(hInfo.front)
                {
                    n1 = 1.0f;
                    n2 = ior;
                }
                // from this to air, may happen internal reflection
                else
                {
                    n1 = ior;
                    n2 = 1.0f;
                }
                
                Vec3f NCrossV = N.Cross(V);
                float sinTheta = NCrossV.Length();
                float sinRefract = n1 * sinTheta / n2;
                float cosRefract = sqrtf(1.0f - sinRefract * sinRefract);
                
                // no internal reflection
                if(sinRefract < 1.0f)
                {
                    float Rs0 = powf((n1 - n2)/(n1 + n2), 2.0f);
                    Rs = Rs0 + (1 - Rs0) * powf((1 - cosTheta), 5.0f);
                    
                    // Generate Refract Ray
                }
            }
            
            // has reflection
            if(reflection.Sum() > 0.0f)
            {
                // only consider front reflect
                if(hInfo.front)
                {
                    // Generate Reflect Ray Check Target
                    Vec3f R = Reflect(V, N);
                    Ray reflectRay;
                    reflectRay.dir = R;
                    reflectRay.p = hInfo.p + N * INTERSECTION_BIAS;
                    
                    HitInfo reflectHitInfo;
//                    GenerateRayForNearestIntersection(<#Node *node#>, <#Ray &ray#>, <#HitInfo &hitinfo#>)
//                    
//                    this->Shade(reflectRay, reflectHitInfo, <#lights#>, <#bounceCount#>)
                }
            }
            
            if(cosTheta < 0)
            {
                continue;
            }
            
            Color iComing = light->Illuminate(hInfo.p + N * INTERSECTION_BIAS, N);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor = iComing * specular * pow(H.Dot(N), glossiness);
            // / cosTheta;
            
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
