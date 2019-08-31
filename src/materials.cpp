#include "materials.h"
#include <math.h>

Color MtlBlinn::Shade(Ray const &ray, const HitInfo &hInfo, const LightList &lights) const
{
    // ray obj space
    // N obj space
    // p obj space
    // depth space independent
    // node space independent
    
    Color result = Color::Black();
    
    for(size_t index = 0; index < lights.size(); index++)
    {
        Light* light = lights[index];
        Color iComing = light->Illuminate(hInfo.p, hInfo.N);
        float cosTheta = Clamp(light->Direction(hInfo.p).Dot(hInfo.N), 0.0f, 1.0f);
        Color colorComing = iComing * cosTheta;
        Color diffuseColor = Color(colorComing * diffuse);
        Vec3f H = (ray.dir.GetNormalized() + light->Direction(hInfo.p)) * 0.5f;
        Color specularColor = specular * pow(H.Dot(hInfo.N), glossiness) / cosTheta;
        result += (diffuseColor + specularColor);
    }
    
    return result;
}

void MtlBlinn::SetViewportMaterial(int subMtlID) const
{
}
