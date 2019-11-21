#pragma once

#include "node.h"
#include "hitinfo.h"
#include "raytracer.h"
#include "objects.h"

class LightComponent 
{
public:
	cy::Color Le();
	float Pdf(const HitInfo& hitInfo, const Vec3f& samplePoint, float distance);
	cy::Color SampleLi(const HitInfo& hitInfo, float& pdf, Vec3f& wi);

	Node* parent = nullptr;
	float intensity = 0.0f;
};