#pragma once
#include "sampler.h"
#include "raytracer.h"
#include "renderimagehelper.h"
#include "constants.h"

#include "utils.h"
#include "materials.h"

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

		auto& hitinfo = hitInfoContext.mainHitInfo;

		auto node = hitInfoContext.mainHitInfo.node;
		
		auto object = node->GetNodeObj();
		MtlBlinn* material = (MtlBlinn*)(node->GetMaterial());

		auto light = node->GetLight();
		if (light != nullptr)
		{
			color += throughput * light->Le();
		}

		position = hitinfo.p;
		normal = hitinfo.N;
		outputDirection = -1.0f * rayContext.cameraRay.dir;
		outputDirection.Normalize();

		Vec3f wi;
		float pdf;
		material->Sample(hitinfo, wi, pdf);

		throughput = throughput * material->EvalBrdf(hitinfo, wi, outputDirection) / pdf;

		// shoot a new ray
		Ray newRay(position + wi * INTERSECTION_BIAS, wi);
		rayContext.cameraRay = newRay;
		rayContext.rightRay = newRay;
		rayContext.topRay = newRay;
		rayContext.hasDiff = false;

		hitInfoContext.Init();

		if (bounces > 3)
		{
			float p = throughput.Max();
			float random = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
			if (random > p)
			{
				break;
			}

			throughput *= (1.0f / p);
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