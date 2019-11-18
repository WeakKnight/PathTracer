#include "lights.h"
#include "raytracer.h"
#include "utils.h"
#include "config.h"

extern Node rootNode;

QuasyMonteCarloCircleSampler* GenLight::CircleAreaLightSampler = new QuasyMonteCarloCircleSampler;

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

Vec3f PointLight::Direction(Vec3f const& p) const 
{ 
	Vec3f top = (position - p).GetNormalized();
	
	Vec3f right;
	Vec3f forward;

	BranchlessONB(top, right, forward);

	float radius = size;
	Vec2f p2d = CircleAreaLightSampler->RandomPointInCircle(size);

	Vec3f shadowRayDir = position - p + p2d.x * right + p2d.y * forward;
	return (-shadowRayDir).GetNormalized();
}

Color PointLight::Illuminate(Vec3f const& p, Vec3f const& N) const
{
	Vec3f top = (position - p).GetNormalized();

	Vec3f right;
	Vec3f forward;

	BranchlessONB(top, right, forward);
	
	float radius = size;

	Vec2f p2d = RandomPointInCircle(size);

	Vec3f shadowRayDir = position - p + p2d.x * right + p2d.y * forward;
	// shadowRayDir.Normalize();

	float shadowFactor = Shadow(Ray(p, shadowRayDir), 1);

	// distance square
	float distanceSquare = (position - p).LengthSquared();
	float distance = sqrt(distanceSquare);
	float fallFactor = 1.0f / (1.0f + 0.09f * distance + 0.032f * distanceSquare);

	if (!LightFallOff)
	{
		fallFactor = 1.0f;
	}

	return shadowFactor * fallFactor * intensity;
}

Color PointLight::GetFallOffIntensity(Vec3f const& x) const
{
	float distanceSquare = (position - x).LengthSquared();
	float distance = sqrt(distanceSquare);

	float fallFactor = 1.0f / (1.0f + 0.09f * distance + 0.032f * distanceSquare);
	
	if (!LightFallOff)
	{
		fallFactor = 1.0f;
	}

	return fallFactor * intensity;
}