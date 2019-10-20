#include "sampler.h"
#include "raytracer.h"

#define COLOR_TOLERANCE 0.001f

extern RenderImage renderImage;

HaltonSampler::HaltonSampler():
sampleNum(32)
{
    int64_t seed = 6000;
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
}

SampleResult HaltonSampler::SamplePixel(int x, int y)
{
    SampleResult result;
    result.x = x;
    result.y = y;
    std::uniform_real_distribution<float> unif(0, ONE_MINUS_EPSILON);
    Vec2f randomOffset = Vec2f(unif(rng), unif(rng));
    
    for(int i = 0; i < sampleNum; i++)
    {
        Vec2f samplerPos = Vec2f(Halton(i, haltonXBase), Halton(i, haltonYBase));
        
        float finalX = samplerPos.x + randomOffset.x;
        if(finalX >= 1.0f)
        {
            finalX -= 1.0f;
        }
        
        float finalY = samplerPos.y + randomOffset.y;
        if(finalY >= 1.0f)
        {
            finalY -= 1.0f;
        }
        
        RayContext rayContext = GenRayContext(GenCameraRay(x, y, finalX, finalY));
        HitInfoContext hitInfoContext;
        
        result.avgColor += (RootTrace(rayContext, hitInfoContext, x, y) / (float)sampleNum);
        result.avgZ = hitInfoContext.mainHitInfo.z;
        result.avgN += (hitInfoContext.mainHitInfo.N / (float)sampleNum);
    }
    
    results.push_back(result);
    
    return result;
}

void HaltonSampler::BeginSampling()
{
    
}
