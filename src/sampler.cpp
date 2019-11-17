#include "sampler.h"
#include "raytracer.h"
#include "renderimagehelper.h"

extern RenderImage renderImage;

HaltonSampler::HaltonSampler():
colorTolerance(0.01f),
sampleNum(32),
minimumSampleNum(4)
{
    results = new std::vector<PixelContext>();
    int64_t seed = 6000;
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
}

PixelContext HaltonSampler::SamplePixel(int x, int y, Vec2f& randomOffset, int index) const
{
    Vec2f samplerPos = Vec2f(Halton(index, haltonXBase) - 0.5f, Halton(index, haltonYBase) - 0.5f);
      
    float finalX = samplerPos.x + randomOffset.x;
    if(finalX >= 0.5f)
    {
        finalX -= 1.0f;
    }
        
    float finalY = samplerPos.y + randomOffset.y;
    if(finalY >= 0.5f)
    {
        finalY -= 1.0f;
    }
        
    RayContext rayContext = GenCameraRayContext(x, y, finalX, finalY);
    HitInfoContext hitInfoContext;
	hitInfoContext.SetAsScreenInfo(x, y);

    Color sampleColor = RootTrace(rayContext, hitInfoContext, x, y);
	PixelContext tempSampleResult;
    tempSampleResult.color = sampleColor;
    tempSampleResult.z = hitInfoContext.mainHitInfo.z;
    tempSampleResult.normal = hitInfoContext.mainHitInfo.N;
        
	auto& resultColor = tempSampleResult.color;
	tempSampleResult.color = Color(powf(resultColor.r, 0.4545f), powf(resultColor.g, 0.4545f), powf(resultColor.b, 0.4545f));

    return tempSampleResult;
}

void HaltonSampler::BeginSampling()
{
    
}
