#include "lights.h"
#include "raytracer.h"
#include "utils.h"

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
	Vec3f top = (position - p).GetNormalized();

	Vec3f randomVector = RandomInUnitSphere().GetNormalized();
	while (randomVector.Dot(top) >= (1.0f - RANDOM_THRESHOLD))
	{
		randomVector = RandomInUnitSphere().GetNormalized();
	}

	Vec3f right = randomVector.Cross(top).GetNormalized();
	assert(right.IsUnit());

	Vec3f forward = right.Cross(top).GetNormalized();
	assert(forward.IsUnit());
	
	float radius = size;

	Vec2f p2d = RandomPointInCircle(size);

	Vec3f shadowRayDir = position - p + p2d.x * right + p2d.y * forward;
	// shadowRayDir.Normalize();

	return Shadow(Ray(p, shadowRayDir), 1) * intensity;
	// return Shadow(Ray(p, position - p), 1) * intensity;
}