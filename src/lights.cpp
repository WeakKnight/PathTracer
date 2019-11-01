#include "lights.h"
#include "raytracer.h"

extern Node rootNode;

void GenLight::SetViewportParam(int lightID, ColorA ambient, ColorA intensity, Vec4f pos ) const
{
    
}

float GenLight::Shadow(Ray ray, float t_max)
{
    if (GenerateRayForAnyIntersection(ray, t_max))
    {
        return 0.0f;
    }
    return 1.0f;
}


Color PointLight::Illuminate(Vec3f const& p, Vec3f const& N) const
{
	return Shadow(Ray(p, position - p), 1) * intensity;
}