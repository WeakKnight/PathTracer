#pragma once

#include "node.h"
#include "hitinfo.h"
#include "raytracer.h"
#include "objects.h"

class LightComponent 
{
public:
	cy::Color Le();
	float Pdf(const HitInfo& hitInfo, const Interaction& sampleInteraction, float distance);
	float Pdf(const HitInfo& hitInfo, const Vec3f& wi);
	cy::Color SampleLi(const HitInfo& hitInfo, float& pdf, Vec3f& wi);

	Node* parent = nullptr;
	cy::Color intensity = cy::Color::Black();
};