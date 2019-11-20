#include "sampler.h"
#include "raytracer.h"
#include "renderimagehelper.h"
#include "spdlog/spdlog.h"

extern RenderImage renderImage;

HaltonSampler::HaltonSampler()
{
    int64_t seed = 6000;
    std::seed_seq ss{uint32_t(seed & 0xffffffff), uint32_t(seed>>32)};
    rng.seed(ss);
}

RayContext HaltonSampler::SamplePixel(int x, int y, Vec2f& randomOffset, int index) const
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
        
    return GenCameraRayContext(x, y, finalX, finalY);
}

