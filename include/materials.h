#pragma once

#include "cyVector.h"
#include "cyMatrix.h"
#include "cyColor.h"

#include "scene.h"

class Material : public ItemBase
{
public:

	virtual void Sample(const HitInfo& hInfo, Vec3f& wi, const Vec3f& wo, float& probability)
	{

	}

	virtual Color EvalBrdf(const HitInfo& hInfo, const Vec3f& wi, const Vec3f& wo)
	{
		return Color::Black();
	}
};