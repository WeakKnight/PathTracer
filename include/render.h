#pragma once
#include "sampler.h"
#include "raytracer.h"
#include "renderimagehelper.h"
#include "constants.h"

extern Node rootNode;
extern TexturedColor environment;

PixelContext RenderPixel(RayContext& rayContext, int x, int y)
{
	HitInfoContext hitInfoContext;
	hitInfoContext.SetAsScreenInfo(x, y);

	Color color = Color::Black();
	Color throughput = Color(1.0f, 1.0f, 1.0f);
	Vec3f position;
	Vec3f normal;
	Vec3f outputDirection;

	for (int bounces = 0; bounces < IndirectLightBounceCount; bounces++)
	{
		bool sthTraced = TraceNode(hitInfoContext, rayContext, &rootNode, HIT_FRONT_AND_BACK);
		if (!sthTraced)
		{
			color += throughput * environment.SampleEnvironment(rayContext.cameraRay.dir);
			break;
		}

		auto node = hitInfoContext.mainHitInfo.node;
		
		auto object = node->GetNodeObj();
		auto material = node->GetMaterial();

		auto light = node->GetLight();
		if (light != nullptr)
		{
			
		}
	}

	PixelContext tempSampleResult;
	tempSampleResult.color = color;
	tempSampleResult.z = hitInfoContext.mainHitInfo.z;
	tempSampleResult.normal = hitInfoContext.mainHitInfo.N;

	auto& resultColor = tempSampleResult.color;

	// Exposure tone mapping
	Color mappedColor =
		//resultColor;
		Color(
			1.0f - exp(-resultColor.r * exposure),
			1.0f - exp(-resultColor.g * exposure),
			1.0f - exp(-resultColor.b * exposure));

	// gamma correction
	tempSampleResult.color = Color(powf(mappedColor.r, 0.4545f), powf(mappedColor.g, 0.4545f), powf(mappedColor.b, 0.4545f));

	return tempSampleResult;
}