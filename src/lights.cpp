#include "lights.h"
#include "raytracer.h"

extern Node rootNode;

void GenLight::SetViewportParam(int lightID, ColorA ambient, ColorA intensity, Vec4f pos ) const
{
    
}

float GenLight::Shadow(Ray ray, float t_max)
{
    HitInfo hitinfo;
    if (GenerateRayForNearestIntersection(ray, hitinfo))
    {
        if(hitinfo.z < t_max)
        {
            return 0.0f;
        }
    }
    return 1.0f;
}


