#include "materials.h"
#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"
#include <assert.h>

using namespace cy;

Color MtlBlinn::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const
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
            colorComing = light->Illuminate(hInfo.p, hInfo.N);
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor =
            Color::Black();
            
            result += (diffuseColor + specularColor);
        }
        else
        {
            Vec3f lightDir = -1.0f * light->Direction(hInfo.p);
            assert(lightDir.IsUnit());
            
            assert(hInfo.N.IsUnit());
            
            Vec3f H = (-1.0f * ray.dir.GetNormalized() + lightDir).GetNormalized();
            assert(H.IsUnit());
            
            float cosTheta = lightDir.Dot(hInfo.N);
            
            if(cosTheta < 0)
            {
                continue;
            }
            
            Color iComing = light->Illuminate(hInfo.p, hInfo.N);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor = iComing * specular * pow(H.Dot(hInfo.N), glossiness);
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

Color MtlPhong::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const
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
        
        auto reflect = [](Vec3f I, Vec3f N){
            return I - 2.0f * N.Dot(I) * N;
        };
        
        Vec3f R = reflect(lightDir, hInfo.N);
        Vec3f V = -1.0f * ray.dir.GetNormalized();
        
        float cosTheta = lightDir.Dot(hInfo.N);
        if(cosTheta < 0)
        {
            cosTheta = 0;
        }
        
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
            Color iComing = light->Illuminate(hInfo.p, hInfo.N);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor = iComing * specular * pow(V.Dot(R), glossiness);
            // / cosTheta;
            
            result += (diffuseColor + specularColor);
        }
    }
    
    return result;
}

void MtlPhong::SetViewportMaterial(int subMtlID) const
{
}
