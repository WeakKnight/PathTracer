#pragma once

#include "cyIrradianceMap.h"
#include "raytracer.h"
#include "sampler.h"
#include "renderimagehelper.h"
#include "spdlog/spdlog.h"
#include "config.h"

extern Node rootNode;
extern LightList lights;
extern TexturedColor background;
extern TexturedColor environment;
extern Color24* irradianceCachePixels;
extern RenderImage renderImage;

class IrradianceCacheMap : public IrradianceMapColorZNormal
{
public:
	IrradianceCacheMap(float _thresholdColor = 0.05, float _thresholdZ = 4.0f, float _thresholdN = 0.9f) :IrradianceMapColorZNormal(_thresholdColor, _thresholdZ, _thresholdN)
	{

	}

	virtual void ComputePoint(ColorZNormal& data, float x, float y, int threadID)
	{
		//// spdlog::debug("compute point {}, {}", x, y);
		//Quasy2DSampler sampler;
		//Color result = Color::Black();
		//float resultZ = 0.0f;
		//bool firstZ = false;

		//Vec3f resultN = Vec3f(0.0f, 0.0f, 0.0f);
		//bool firstN = false;

		//static const float factor = 1.0f / (float)IrradianceGISampleCount;
		//bool hasCached = false;

		//for (int i = 0; i < IrradianceGISampleCount; i++)
		//{
		//	auto randomOffset = sampler.GenRandom2DVector();
		//	auto clampedOffset = Vec2f(randomOffset.x - 0.5f, randomOffset.y - 0.5f);

		//	RayContext rayContext = GenCameraRayContext(x, y, clampedOffset.x, clampedOffset.y);
		//	HitInfoContext hitInfoContext;

		//	Color sampleColor = RootTrace(rayContext, hitInfoContext, x, y);
		//	HitInfo& hitInfo = hitInfoContext.mainHitInfo;

		//	bool sthTraced = TraceNode(hitInfoContext, rayContext, &rootNode, HIT_FRONT_AND_BACK);

		//	if (sthTraced)
		//	{
		//		Color shadingResult = hitInfo.node->GetMaterial()->IndirectLightShade(hitInfoContext.mainHitInfo.N, rayContext, hitInfoContext, lights, RefractionBounceCount, IndirectLightBounceCount);
		//		result += factor * shadingResult;
		//		hasCached = true;
		//	}

		//	resultZ += factor * hitInfoContext.mainHitInfo.z;
		//	resultN += hitInfoContext.mainHitInfo.N;
		//}

		//resultN.Normalize();

		//if (hasCached)
		//{
		//	if (irradianceCachePixels != nullptr)
		//	{
		//		RenderImageHelper::SetIrradianceCache(irradianceCachePixels, renderImage, x, y);
		//	}
		//}

		//data.c = result;
		//data.z = resultZ;
		//data.N = resultN;
	}
};