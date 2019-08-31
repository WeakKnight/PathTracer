#include "materials.h"
#include <math.h>
#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"

using namespace cy;

Color MtlBlinn::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const
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
        float cosTheta = lightDir.Dot(hInfo.N);
        if(cosTheta < 0)
        {
            cosTheta = 0;
        }
        
        Vec3f H = (-1.0f * ray.dir.GetNormalized() + lightDir).GetNormalized();
        
        if(light->IsAmbient())
        {
            colorComing = light->Illuminate(hInfo.p, hInfo.N);
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            
            
            Color specularColor = colorComing * specular * pow(H.Dot(hInfo.N), glossiness);
            // / cosTheta;
            
            result += (diffuseColor + specularColor);
        }
        else
        {
            Color iComing = light->Illuminate(hInfo.p, hInfo.N);
            colorComing = iComing * cosTheta;
            
            Color diffuseColor = Color(colorComing * diffuse);
            
            Color specularColor = iComing * specular * pow(H.Dot(hInfo.N), glossiness);
            // / cosTheta;
            
            result += (diffuseColor + specularColor);
        }
    }
    
    return result;
}

void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}
