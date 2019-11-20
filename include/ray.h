#pragma once
#include "cyMatrix.h"
#include "cyVector.h"
#include "constants.h"

using namespace cy;

class Ray
{
public:
	Vec3f p;
	Vec3f dir;

	Ray() {}
	Ray(Vec3f const& _p, Vec3f const& _dir) : p(_p), dir(_dir) {}
	Ray(Ray const& r) : p(r.p), dir(r.dir) {}
	void Normalize() { dir.Normalize(); }
};


struct RayContext
{
	Ray cameraRay;
	// Ray Differential
	Ray rightRay;
	Ray topRay;
	float delta;
	bool hasDiff = true;

	RayContext() {}
	RayContext(RayContext const& r) :
		cameraRay(r.cameraRay),
		rightRay(r.rightRay),
		topRay(r.topRay),
		delta(r.delta)
	{

	}
};