#include "sampler.h"
#include "raytracer.h"
#include "renderimagehelper.h"

extern RenderImage renderImage;

HaltonSampler::HaltonSampler():
colorTolerance(0.01f),
sampleNum(32),
minimumSampleNum(4)
{
    results = new std::vector<SampleResult>();
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
    
    std::vector<SampleResult> tempSampleResults;
    
    for(size_t i = 0; i < sampleNum; i++)
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
        
        RayContext rayContext = GenCameraRayContext(x, y, finalX, finalY);
        HitInfoContext hitInfoContext;
        
        Color sampleColor = RootTrace(rayContext, hitInfoContext, x, y);
        SampleResult tempSampleResult;
        tempSampleResult.avgColor = sampleColor;
        tempSampleResult.avgZ = hitInfoContext.mainHitInfo.z;
        tempSampleResult.avgN = hitInfoContext.mainHitInfo.N;
        
        tempSampleResults.push_back(tempSampleResult);
        
        if(!ContinueSamplingCondition(tempSampleResults))
        {
            break;
        }
    }
    
    RenderImageHelper::SetSampleNum(renderImage, x, y, tempSampleResults.size());
    
    for(size_t i = 0; i < tempSampleResults.size(); i++)
    {
        result.avgColor += (tempSampleResults[i].avgColor / (float)tempSampleResults.size());
        result.avgN += (tempSampleResults[i].avgN / (float)tempSampleResults.size());
        result.avgZ = tempSampleResults[i].avgZ;
    }
    
    // results->push_back(result);
    
    return result;
}

void HaltonSampler::BeginSampling()
{
    
}

bool HaltonSampler::ContinueSamplingCondition(std::vector<SampleResult>& sampleResult)
{
    if(sampleResult.size() < this->minimumSampleNum)
    {
        return true;
    }
    
    Color meanColor = Color::Black();
    for(size_t i = 0; i < sampleResult.size(); i ++)
    {
        meanColor += (sampleResult[i].avgColor / sampleResult.size());
    }
    
    for(size_t i = 0; i < sampleResult.size(); i ++)
    {
        auto delta = (sampleResult[i].avgColor - meanColor);
        delta.Abs();
        
        if(delta.Sum() >= colorTolerance)
        {
            return true;
        }
    }
    
    return false;
}
