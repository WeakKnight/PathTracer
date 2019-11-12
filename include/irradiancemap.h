#pragma once

#include "cyIrradianceMap.h"
#include "raytracer.h"
#include "sampler.h"
#include "renderimagehelper.h"
#include "spdlog/spdlog.h"

extern Node rootNode;
constexpr int IrradianceGISampleCount = 4;
extern LightList lights;
extern TexturedColor background;
extern TexturedColor environment;
extern Color24* irradianceCachePixels;
extern RenderImage renderImage;

class IrradianceCacheMap : public IrradianceMapColorZNormal
{
public:
	virtual void ComputePoint(ColorZNormal& data, float x, float y, int threadID)
	{
		spdlog::debug("compute point {}, {}", x, y);
		Quasy2DSampler sampler;
		Color result = Color::Black();
		float resultZ = 0.0f;
		bool firstZ = false;

		Vec3f resultN = Vec3f(0.0f, 0.0f, 0.0f);
		bool firstN = false;

		static const float factor = 1.0f / (float)IrradianceGISampleCount;
		bool hasCached = false;

		for (int i = 0; i < IrradianceGISampleCount; i++)
		{
			auto randomOffset = sampler.GenRandom2DVector();
			auto clampedOffset = Vec2f(randomOffset.x - 0.5f, randomOffset.y - 0.5f);

			RayContext rayContext = GenCameraRayContext(x, y, clampedOffset.x, clampedOffset.y);
			HitInfoContext hitInfoContext;

			Color sampleColor = RootTrace(rayContext, hitInfoContext, x, y);
			HitInfo& hitInfo = hitInfoContext.mainHitInfo;

			bool sthTraced = TraceNode(hitInfoContext, rayContext, &rootNode, HIT_FRONT_AND_BACK);

			if (sthTraced)
			{
				Color shadingResult = hitInfo.node->GetMaterial()->IndirectLightShade(rayContext, hitInfoContext, lights, 1);
				result += factor * shadingResult;
				hasCached = true;
			}

			if (!firstZ)
			{
				firstZ = true;
				resultZ = hitInfoContext.mainHitInfo.z;
			}

			if (!firstN)
			{
				firstN = true;
				resultN = hitInfoContext.mainHitInfo.N;
			}
		}

		if (hasCached)
		{
			if (irradianceCachePixels != nullptr)
			{
				int width = renderImage.GetWidth();
				int height = renderImage.GetHeight();
				if (x < width && y < height)
				{
					irradianceCachePixels[(int)(x)+(int)(y)*width] = Color24(255.0f, 255.0f, 255.0f);
				}
			}
		}

		data.c = result;
		data.z = resultZ;
		data.N = resultN;
	}
};